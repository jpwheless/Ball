#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <SFML/Graphics.hpp>
#include <SFGUI/SFGUI.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "quad.hpp"
#include "barrier.hpp"
#include "particles.hpp"
#include "input.hpp"

//==============================

#define MAX_TICKTIME 0.001666
#define TICKTIME_AVGFILT 0.05
#define SCALEFACT_AVGFILT 5.0

#define DEFAULT_RES_X 1500
#define DEFAULT_RES_Y 900

#define DEFAULT_LIN_GRAV 1000.0

#define DEFAULT_NUM_BALLS 1000

#define DIA_SMALL 0
#define DIA_MED 1
#define DIA_LARGE 2

#define DENSITY_LIGHT 0
#define DENSITY_MED 1
#define DENSITY_HEAVY 2

#define MULTITHREAD true

//==============================

static std::mutex pauseMutex1;
static std::mutex pauseMutex2;
static std::condition_variable pauseCV1;
static std::condition_variable pauseCV2;

namespace z {

class Simulation {
private:	
	sf::Font calibri;
	sf::Text fps;
	
	// SFGUI
	sfg::SFGUI sfguiW;
	sfg::Window::Ptr guiWindow;
	sfg::CheckButton::Ptr cbStickyness;
	sfg::CheckButton::Ptr cbCollision;
	sfg::CheckButton::Ptr cbGravity;
	sfg::CheckButton::Ptr cbBoundCeiling;
	sfg::CheckButton::Ptr cbBoundWalls;
	sfg::CheckButton::Ptr cbBoundFloor;
	sfg::ToggleButton::Ptr bPause;
	sfg::Button::Ptr bDebug;
	sfg::Button::Ptr bClear;
	sfg::Button::Ptr bStop;
	sfg::ProgressBar::Ptr scaleBar;
	sfg::Scale::Ptr scaleScale;
	sfg::Adjustment::Ptr scaleAdjustment;
	sfg::ComboBox::Ptr densityCombo;
	sfg::ComboBox::Ptr diameterCombo;
	
	sfg::RadioButton::Ptr mouseFuncErase; // Erase
	sfg::RadioButton::Ptr mouseFuncDrag; // Drag
	sfg::RadioButton::Ptr mouseFuncPaint; // Paint
	sfg::RadioButton::Ptr mouseFuncShoot; // Shoot
	sfg::RadioButton::Ptr mouseFuncPlaceBH; // Place Blackhole
	sfg::RadioButton::Ptr mouseFuncControlBH; // Control Blackhole
	sfg::RadioButton::Ptr shootFuncClick;
	sfg::RadioButton::Ptr shootFuncDrag;
	sfg::CheckButton::Ptr bhPermCheckButton;
	sfg::CheckButton::Ptr eraseFuncCheckButton;
	sfg::CheckButton::Ptr paintOvrCheckButton;
	sfg::CheckButton::Ptr paintForceCheckButton;
	
	sf::Clock clockP;
	sf::Time elapsedTimeP;
	sf::Clock clockD;
	sf::Time elapsedTimeD;
	double tickTime, tickTimeActual;
	double scaleFactor, frameRateP, frameRateD;
	double tickTimeMax, scaleFactorM;
	
	bool running;
	bool debugRead;
	
	// Threads
	std::thread* drawThread;
	std::thread* calcPhysicsThread1;
	std::thread* calcPhysicsThread2;
	
	z::SpinningBarrier rendezvous1 = z::SpinningBarrier(2);
	z::SpinningBarrier rendezvous2 = z::SpinningBarrier(2);
	z::SpinningBarrier rendezvous3 = z::SpinningBarrier(2);
			
	std::atomic<int>* loadBalance1 = new std::atomic<int>;
	std::atomic<int>* loadBalance2 = new std::atomic<int>;
	std::atomic<int>* loadBalance3 = new std::atomic<int>;
	std::atomic<bool>* finishFlag1 = new std::atomic<bool>;
	std::atomic<bool>* finishFlag2 = new std::atomic<bool>;
	std::atomic<bool>* finishFlag3 = new std::atomic<bool>;
	std::atomic<bool>* threadsPaused = new std::atomic<bool>;

	///////////////////
	// GUI Functions //
	///////////////////
	
	void buttonCollision() {
		particles->particleCollisions = cbCollision->IsActive();
	}
	void buttonStickyness() {
		particles->particleStickyness = cbStickyness->IsActive();
	}
	void buttonGravity() {
		particles->linGravity = (cbGravity->IsActive())?DEFAULT_LIN_GRAV:0.0;
	}
	void buttonBoundCeiling() {
		particles->boundCeiling = cbBoundCeiling->IsActive();
	}
	void buttonBoundFloor() {
		particles->boundFloor = cbBoundFloor->IsActive();
	}
	void buttonBoundWalls() {
		particles->boundWalls = cbBoundWalls->IsActive();
	}
	void buttonDebug() {
		for (unsigned int i = 0; i < particles->ballV.size(); i++) {
			std::cout << "Particle " << particles->ballV[i]->getID() << ": ";
			if (particles->ballV[i]->alive) {
				std::cout << "Vel = " << sqrt(pow(particles->ballV[i]->xVel, 2.0) + pow(particles->ballV[i]->yVel, 2.0));
				std::cout <<", x = " << particles->ballV[i]->x << ", y = " << particles->ballV[i]->y << "\n\tLevel: ";
				std::cout << particles->ballV[i]->quadResidence->level << ", ChildNum: " << particles->ballV[i]->quadResidence->childNum;
				std::cout	<< ", xMin, xMax, yMin, yMax: " << particles->ballV[i]->quadResidence->xMin << "," << particles->ballV[i]->quadResidence->xMax << "," << particles->ballV[i]->quadResidence->yMin << "," << particles->ballV[i]->quadResidence->yMax << "\n";
				particles->ballV[i]->updateBounds();
				std::cout << "\t\t\txMin, xMax, yMin, yMax: " << particles->ballV[i]->xMin << "\t" << particles->ballV[i]->xMax << "\t" << particles->ballV[i]->yMin << "\t" << particles->ballV[i]->yMax << "\n";
				std::cout << "\t\tBall points to Quad Residence: " << ((particles->ballV[i]->quadResidence->checkIfResident(particles->ballV[i]->getID(), false))?"True":"False") << "\n";
			}
			else {
				std::cout << "Inactive\n";
			}
		}
		std::cout << "\n";
		particles->quadTree->printParams();
		std::cout << "\n";
	}
	void buttonPause() {
		if (bPause->IsActive()) {
			*threadsPaused = true;
			bPause->SetLabel("Resume Sim");
		}
		else {
			*threadsPaused = false;
			pauseCV1.notify_one();
			pauseCV2.notify_one();
			bPause->SetLabel("Pause Sim");
		}
	}
	void buttonClear() {
		particles->clearParticles();
	}
	void buttonStop() {
		particles->zeroVel();
	}
	void buttonMouseSelect() {
		if(mouseFuncErase->IsActive()) input->mouseMode = 1;
		else if(mouseFuncDrag->IsActive()) input->mouseMode = 2;
		else if(mouseFuncPaint->IsActive()) input->mouseMode = 3;
		else if(mouseFuncShoot->IsActive()) input->mouseMode = 4;
		else if(mouseFuncPlaceBH->IsActive()) input->mouseMode = 5;
		else if(mouseFuncControlBH->IsActive()) input->mouseMode = 6;
		input->modeChanged = true;
	}
	void buttonShootSelect() {
		mouseFuncShoot->SetActive(true);
		if(shootFuncClick->IsActive()) input->shootMode = 1;
		else if(shootFuncDrag->IsActive()) input->shootMode = 2;
	}
	void buttonPermanence() {
		input->bhPermanent = bhPermCheckButton->IsActive();
		if (input->bhPermanent) {
			particles->bhV[0].active = true;
			particles->bhV[0].xMove = resX/2.0;
			particles->bhV[0].yMove = resY/2.0;
		}
		else {
			particles->bhV[0].active = false;
		}
	}
	void buttonEraseFunc() {
		if (eraseFuncCheckButton->IsActive()) {
			input->eraseFuncOnly = true;
			mouseFuncErase->SetActive(true);
		}
		else input->eraseFuncOnly = false;
	}
	void buttonPaintOvr() {
		if (paintOvrCheckButton->IsActive()) {
			input->paintOvr = true;
			paintForceCheckButton->SetActive(false);
		}
		else input->paintOvr = false;
		mouseFuncPaint->SetActive(true);
	}
	void buttonPaintForce() {
		if (paintForceCheckButton->IsActive()) {
			input->paintForce = true;
			paintOvrCheckButton->SetActive(false);
		}
		else input->paintForce = false;
		mouseFuncPaint->SetActive(true);
	}
	void scaleTimeAdj() {
		scaleFactorM = scaleAdjustment->GetValue();
	}
	void diameterComboFunc() {
		int temp = diameterCombo->GetSelectedItem();
		if (temp >= 0 && temp <= 3) input->newBallDia = temp;
		else {
			input->newBallDia = 0;
			diameterCombo->SelectItem(0);
		}
	}
	void densityComboFunc() {
		int temp = densityCombo->GetSelectedItem();
		if (temp >= 0 && temp <= 3) input->newBallDensity = temp;
		else {
			input->newBallDensity = 0;
			densityCombo->SelectItem(0);
		}
	}
	
	////////////////////
	// Initialization //
	////////////////////
	
	// Call after initGUI()
	void initSFML() {
		sf::Vector2f requisition = guiWindow->GetRequisition();
		{
			int temp = requisition.y;
			requisition.y = std::max(resY, temp);
		}
		guiWindow->SetRequisition(requisition);
		mainWindow = new sf::RenderWindow(sf::VideoMode(resX + requisition.x, resY), 
			"Particles!", sf::Style::Titlebar|sf::Style::Close, 
			sf::ContextSettings(24, 8, 8, 3, 0)); // Set openGL parameters
		
		mainWindow->setFramerateLimit(120);
		mainWindow->setVerticalSyncEnabled(false);

		input->mainWindow = mainWindow;
		
		// Debugging/fps readout
		// Load font from system font folder directory
		char* windir = std::getenv("windir");
		if (windir) {
			std::string windirS(windir);
			if(calibri.loadFromFile(windirS + "\\fonts\\calibri.ttf")) {
				debugRead = true;
				fps.setFont(calibri);
				fps.setCharacterSize(15);
				fps.setColor(sf::Color::White);
			}
		}
		
		cbCollision->SetActive(particles->particleCollisions);
		cbStickyness->SetActive(particles->particleStickyness);
		cbGravity->SetActive(particles->linGravity > 0.0);
		cbBoundCeiling->SetActive(particles->boundCeiling);
		cbBoundWalls->SetActive(particles->boundWalls);
		cbBoundFloor->SetActive(particles->boundFloor);

		bhPermCheckButton->SetActive(input->bhPermanent);
		paintOvrCheckButton->SetActive(input->paintOvr);
		paintForceCheckButton->SetActive(input->paintForce);
		
		scaleAdjustment->SetValue(1.0);
	}

	void initGUI() {
		guiWindow = sfg::Window::Create();
		guiWindow->SetStyle(guiWindow->GetStyle() ^ sfg::Window::TITLEBAR);
		guiWindow->SetStyle(guiWindow->GetStyle() ^ sfg::Window::RESIZE);
		
		auto boxMain = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.0);
		auto boxSim = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.0);
		auto boxParam = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.0);
		auto boxParticles = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.0);
		auto boxMouse = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.0);
		
		auto label1 = sfg::Label::Create();
		label1->SetText("Sim Parameters");
		auto label2 = sfg::Label::Create();
		label2->SetText("Mouse Function");
		auto label3 = sfg::Label::Create();
		label3->SetText("Time Scaling (Actual)");
		auto label4 = sfg::Label::Create();
		label4->SetText("Time Scaling (Max)");
		auto label5 = sfg::Label::Create();
		label5->SetText("Function Particles");
		auto instructions = sfg::Label::Create();
		instructions->SetText("Use the scroll wheel along with CTRL or SHIFT to modify mouse functions.");
		instructions->SetLineWrap(true);
		
		auto alignment1 = sfg::Alignment::Create();
		alignment1->SetScale(sf::Vector2f(0.0f, 0.0f ));
		alignment1->SetAlignment(sf::Vector2f(0.0f, .0f));
		auto alignment2 = sfg::Alignment::Create();
		alignment2->SetScale(sf::Vector2f(0.0f, 0.0f ));
		alignment2->SetAlignment(sf::Vector2f(0.0f, .0f));
		auto alignment3 = sfg::Alignment::Create();
		alignment3->SetScale(sf::Vector2f(0.0f, 0.0f ));
		alignment3->SetAlignment(sf::Vector2f(0.0f, .0f));

		auto separatorh1 = sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL);
		auto separatorh2 = sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL);
		auto separatorh3 = sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL);
		auto separatorh4 = sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL);
		
		// Button functions
		cbCollision = sfg::CheckButton::Create("Collision");
		cbCollision->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonCollision, this));
		
		cbStickyness = sfg::CheckButton::Create("Stickyness");
		cbStickyness->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonStickyness, this));
				
		cbGravity = sfg::CheckButton::Create("Linear Gravity");
		cbGravity->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonGravity, this));
		
		cbBoundCeiling = sfg::CheckButton::Create("Ceiling");
		cbBoundCeiling->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonBoundCeiling, this));
		
		cbBoundWalls = sfg::CheckButton::Create("Walls");
		cbBoundWalls->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonBoundWalls, this));
		
		cbBoundFloor = sfg::CheckButton::Create("Floor");
		cbBoundFloor->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonBoundFloor, this));
		
		bPause = sfg::ToggleButton::Create("Pause Sim");
		bPause->GetSignal( sfg::Widget::OnLeftClick).Connect(std::bind(&z::Simulation::buttonPause, this));
		
		bDebug = sfg::Button::Create("Debug Print");
		bDebug->GetSignal(sfg::Widget::OnMouseLeftRelease).Connect(std::bind(&z::Simulation::buttonDebug, this));
		
		bClear = sfg::Button::Create("Clear Screen");
		bClear->GetSignal(sfg::Widget::OnLeftClick).Connect(std::bind(&z::Simulation::buttonClear, this));
		
		bStop = sfg::Button::Create("Zero Velocities");
		bStop->GetSignal( sfg::Widget::OnLeftClick).Connect(std::bind(&z::Simulation::buttonStop, this));

		scaleBar = sfg::ProgressBar::Create();
		scaleScale = sfg::Scale::Create(sfg::Scale::Orientation::HORIZONTAL);
		scaleScale->SetRequisition(sf::Vector2f(100.0, 20.0));
		
		scaleAdjustment = scaleScale->GetAdjustment();
		scaleAdjustment->SetLower(0.01);
		scaleAdjustment->SetUpper(1.0);
		scaleAdjustment->SetMinorStep(0.01);
		scaleAdjustment->SetMajorStep(0.10);
		scaleAdjustment->GetSignal(sfg::Adjustment::OnChange).Connect(std::bind(&z::Simulation::scaleTimeAdj, this));
		
		mouseFuncErase = sfg::RadioButton::Create("Erase");
		mouseFuncDrag = sfg::RadioButton::Create("Drag", mouseFuncErase->GetGroup());
		mouseFuncPaint = sfg::RadioButton::Create("Paint", mouseFuncErase->GetGroup());
		mouseFuncShoot = sfg::RadioButton::Create("Shoot", mouseFuncErase->GetGroup());
		mouseFuncPlaceBH = sfg::RadioButton::Create("Place Blackhole", mouseFuncErase->GetGroup());
		mouseFuncControlBH = sfg::RadioButton::Create("Control Blackhole", mouseFuncErase->GetGroup());
		mouseFuncErase->SetActive(true);
		mouseFuncErase->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		mouseFuncDrag->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		mouseFuncPaint->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		mouseFuncShoot->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		mouseFuncPlaceBH->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		mouseFuncControlBH->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		
		shootFuncClick = sfg::RadioButton::Create("Click");
		shootFuncDrag = sfg::RadioButton::Create("Drag", shootFuncClick->GetGroup());
		shootFuncClick->SetActive(true);
		shootFuncClick->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonShootSelect, this));
		shootFuncDrag->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonShootSelect, this));
		
		bhPermCheckButton = sfg::CheckButton::Create("Permanent");
		bhPermCheckButton->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonPermanence, this));
		
		eraseFuncCheckButton = sfg::CheckButton::Create("Only Function Particles");
		eraseFuncCheckButton->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonEraseFunc, this));
		
		paintOvrCheckButton = sfg::CheckButton::Create("Overwrite");
		paintOvrCheckButton->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonPaintOvr, this));
		paintForceCheckButton = sfg::CheckButton::Create("Force");
		paintForceCheckButton->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonPaintForce, this));
		
		densityCombo = sfg::ComboBox::Create();
		densityCombo->AppendItem("Low Density");
		densityCombo->AppendItem("Med Density");
		densityCombo->AppendItem("High Density");
		diameterCombo = sfg::ComboBox::Create();
		diameterCombo->AppendItem("Diameter = 10");
		diameterCombo->AppendItem("Diameter = 20");
		diameterCombo->AppendItem("Diameter = 40");
		
		densityCombo->SelectItem(1);
		diameterCombo->SelectItem(0);
		densityCombo->GetSignal(sfg::ComboBox::OnSelect).Connect(std::bind(&z::Simulation::densityComboFunc, this));
		diameterCombo->GetSignal(sfg::ComboBox::OnSelect).Connect(std::bind(&z::Simulation::diameterComboFunc, this));
		
		auto fixed1 = sfg::Fixed::Create();
		fixed1->Put(paintOvrCheckButton, sf::Vector2f(10.0, 0.0));
		fixed1->Put(paintForceCheckButton, sf::Vector2f(10.0, 20.0));
		
		auto fixed2 = sfg::Fixed::Create();
		fixed2->Put(shootFuncClick, sf::Vector2f(10.0, 0.0));
		fixed2->Put(shootFuncDrag, sf::Vector2f(10.0, 20.0));
		
		auto fixed3 = sfg::Fixed::Create();
		fixed3->Put(bhPermCheckButton, sf::Vector2f(10.0, 0.0));
		
		auto fixed4 = sfg::Fixed::Create();
		fixed4->Put(eraseFuncCheckButton, sf::Vector2f(10.0, 0.0));
		
		boxSim->Pack(bPause);
		boxSim->Pack(label3);
		boxSim->Pack(scaleBar);
		boxSim->Pack(label4);
		boxSim->Pack(scaleScale);
		boxSim->Pack(bClear);
		boxSim->Pack(bStop);
		boxSim->Pack(bDebug);
		
		alignment2->Add(label1);
		boxParam->Pack(alignment2, false, true);
		boxParam->Pack(cbCollision);
		boxParam->Pack(cbStickyness);
		boxParam->Pack(cbGravity);
		boxParam->Pack(cbBoundCeiling);
		boxParam->Pack(cbBoundWalls);
		boxParam->Pack(cbBoundFloor);
				
		alignment1->Add(label5);
		boxParticles->Pack(alignment1, false, true);
		boxParticles->Pack(diameterCombo);
		boxParticles->Pack(densityCombo);
		
		alignment3->Add(label2);
		boxMouse->Pack(alignment3, false, true);
		boxMouse->Pack(mouseFuncErase);
		boxMouse->Pack(fixed4, false, true);
		boxMouse->Pack(mouseFuncDrag);
		boxMouse->Pack(mouseFuncPaint);
		boxMouse->Pack(fixed1, false, true);
		boxMouse->Pack(mouseFuncShoot);
		boxMouse->Pack(fixed2, false, true);
		boxMouse->Pack(mouseFuncPlaceBH);
		boxMouse->Pack(mouseFuncControlBH);
		boxMouse->Pack(fixed3, false, true);
				
		boxMain->Pack(boxSim, false, true);
		boxMain->Pack(separatorh1, false, true);
		boxMain->Pack(boxParam, false, true);
		boxMain->Pack(separatorh2, false, true);
		boxMain->Pack(boxParticles, false, true);
		boxMain->Pack(separatorh3, false, true);
		boxMain->Pack(boxMouse, false, true);
		boxMain->Pack(separatorh4, false, true);
		boxMain->Pack(instructions, false, true);
		
		guiWindow->Add(boxMain);
		
		guiWindow->SetPosition(sf::Vector2f(resX, 0));
	}
		
public:
	int resX, resY;
	sf::RenderWindow* mainWindow;
	z::Input *input;
	
	z::Particles *particles;
	
	Simulation() {
		srand(static_cast<unsigned>(time(0)));
		loadParams();
	}

	~Simulation() {
		// Clean up
		delete mainWindow;
		delete drawThread;
		delete calcPhysicsThread1;
		delete calcPhysicsThread2;
		delete particles;
		delete input;
		delete loadBalance1;
		delete loadBalance2;
		delete finishFlag1;
		delete finishFlag2;
	}
	
	void loadParams() {
		// Default simulation parameters
		resX = DEFAULT_RES_X;
		resY = DEFAULT_RES_Y;
		
		particles = new Particles(&resX, &resY, &tickTime, DEFAULT_LIN_GRAV);
		input = new z::Input(particles);
																			
		input->newBallDia = DIA_SMALL;
		input->newBallDensity = DENSITY_MED;

		particles->particleCollisions = true;
		particles->particleStickyness = false;
		particles->boundCeiling = true;
		particles->boundWalls = true;
		particles->boundFloor = true;
		
		particles->createInitBalls(DEFAULT_NUM_BALLS, DIA_SMALL, DENSITY_MED);
		
		/*
		// Open file and read values
		std::ifstream parameterFile("parameters.ini");
		if (parameterFile.is_open()) { // Read parameter file
			
			std::string line;
			
			// Read all lines of file, sorting values into variables
			while(getline(parameterFile, line)) {
				std::istringstream lineSS(line);
				
				std::string paramName;
				std::string param;
				if (std::getline(lineSS, paramName, '=')) {
					if (paramName == "WINDOW_WIDTH") {
						if (std::getline(lineSS, param, ' ')) {
							int temp = stoi(param);
							if (temp >= 100) resX = temp;
						}
					}
					else if (paramName == "WINDOW_HEIGHT") {
						if (std::getline(lineSS, param, ' ')) {
							int temp = stoi(param);
							if (temp >= 100) resY = temp;
						}
					}
					else if (paramName == "LINEAR_GRAVITY") {
						if (std::getline(lineSS, param, ' ')) {
							linGravity = stof(param);
						}
					}
					else if (paramName == "INITIAL_PARTICLES") {
						if (std::getline(lineSS, param, ' ')) {
							int temp = stoi(param);
							if (temp >= 0) particles->initialNumBalls = temp;
						}
					}
					else if (paramName == "INITIAL_PARTICLE_DIAMETER") {
						if (std::getline(lineSS, param, ' ')) {
							double temp = stof(param);
							if (temp >= 1.0) particles->defaultBallDia = temp;
						}
					}
					else if (paramName == "INITIAL_PARTICLE_ATTRACT_RATE") {
						if (std::getline(lineSS, param, ' ')) {
							particles->defaultBallAttrRate = stof(param)*pow(particles->defaultBallDia/2.f, 2.f);;
						}
					}
					else if (paramName == "INITIAL_PARTICLE_ATTRACT_RADIUS") {
						if (std::getline(lineSS, param, ' ')) {
							double temp = stof(param);
							if (temp >= 0) particles->defaultBallAttrRad = temp;
						}
					}
					else if (paramName == "PARTICLE_BOUNDARY") {
						if (std::getline(lineSS, param, ' ')) {
							if (param == "true") particles->particleBoundary = true;
							else if (param == "false") particles->particleBoundary = false;
						}
					}
					else if (paramName == "PARTICLE_COLLISIONS") {
						if (std::getline(lineSS, param, ' ')) {
							if (param == "true") particles->particleCollisions = true;
							else if (param == "false") particles->particleCollisions = false;
						}
					}
					else if (paramName == "PARTICLE_STICKYNESS") {
						if (getline(lineSS, param, ' ')) {
							if (param == "true") particles->particleStickyness = true;
							else if (param == "false") particles->particleStickyness = false;
						}
					}
				}
			}
		}
		parameterFile.close();
		
		// Assume reformatting/creation is necessary
		std::ofstream createParameter("parameters.ini");
		if (createParameter.is_open()) {
			createParameter << "WINDOW_WIDTH=" << resX << "\n";
			createParameter << "WINDOW_HEIGHT=" << resY << "\n";
			createParameter << "LINEAR_GRAVITY=" << linGravity << "\n";
			createParameter << "INITIAL_PARTICLES=" << particles->initialNumBalls << "\n";
			createParameter << "INITIAL_PARTICLE_DIAMETER=" << particles->defaultBallDia << "\n";
			createParameter << "INITIAL_PARTICLE_ATTRACT_RATE=" << particles->defaultBallAttrRate/pow(particles->defaultBallDia/2.f, 2.f) << "\n";
			createParameter << "INITIAL_PARTICLE_ATTRACT_RADIUS=" << particles->defaultBallAttrRad << "\n";
			//createParameter << "PARTICLE_BOUNDARY=" << ((particles->particleBoundary) ? "true" : "false") << "\n";
			createParameter << "PARTICLE_COLLISIONS=" << ((particles->particleCollisions) ? "true" : "false") << "\n";
			createParameter << "PARTICLE_STICKYNESS=" << ((particles->particleStickyness) ? "true" : "false") << "\n";
		}
		createParameter.close();
		
		particles->createInitBalls();
		*/
	}
		
	void launch() {
		running = true;
		*threadsPaused = false;
		drawThread = new std::thread(&Simulation::draw, this);
		calcPhysicsThread1 = new std::thread(&Simulation::calcPhysics1, this);
		if (MULTITHREAD) {
			calcPhysicsThread2 = new std::thread(&Simulation::calcPhysics2, this);
			*loadBalance1 = particles->ballV.size() / 2.0;
			*loadBalance2 = particles->ballV.size() / 2.0;
			*loadBalance3 = particles->ballV.size() / 2.0;
		}
		else {
			*loadBalance1 = 0;
			*loadBalance2 = 0;
			*loadBalance3 = 0;
		}
		
		tickTime = 0.002; // Jump start to avoid physics glitches
		tickTimeActual = tickTime;
		tickTimeMax = MAX_TICKTIME;
		frameRateP = 500;
		frameRateD = 60;
		scaleFactor = 1.0;
		
		clockD.restart();
		clockP.restart();
						
		drawThread->join();
		calcPhysicsThread1->join();
		if (MULTITHREAD) calcPhysicsThread2->join();
	}
	
	/////////////
	// Threads //
	/////////////
	void draw() {
		initGUI();
		initSFML();
				
		sf::Vertex menuDivider[] = {
			sf::Vertex(sf::Vector2f(resX, 0)),
			sf::Vertex(sf::Vector2f(resX, resY))
		};
				
		while (mainWindow->isOpen()) {
				
			elapsedTimeD = clockD.restart();
			frameRateD = 0.05*(1.0/elapsedTimeD.asSeconds()) + frameRateD*(1.0 - 0.05);

			// Handle events
			sf::Event event;
			while (mainWindow->pollEvent(event)) {
				guiWindow->HandleEvent(event);
				switch(event.type) {
					case sf::Event::Closed:
						mainWindow->close();
						running = false;
						*threadsPaused = false;
						pauseCV1.notify_one();
						pauseCV2.notify_one();
						break;
					case sf::Event::LostFocus:
						input->windowFocused = false;
						break;
					case sf::Event::GainedFocus:
						input->windowFocused = true;
						break;
					case sf::Event::MouseWheelMoved:
						input->inputWheel(event);
						break;
					case sf::Event::MouseButtonReleased:
						if (event.mouseButton.button == sf::Mouse::Left) input->mouseReleased = true;
						break;
					case sf::Event::MouseButtonPressed:
						if (event.mouseButton.button == sf::Mouse::Left) input->mousePressed = true;
						break;
					default:
						break;
				}
			}
									
			input->update();
			
			scaleBar->SetFraction(scaleFactor);
			guiWindow->Update(1.0);
			
			mainWindow->clear();
			
			particles->draw(mainWindow);
			input->draw();
			mainWindow->draw(menuDivider, 2, sf::Lines);
						
			// Draw text/gui
			if (debugRead) {
				std::string temp = std::to_string(scaleFactor);
				temp.resize(4);
				fps.setString(std::to_string((int)frameRateP) + "," + temp + "," + std::to_string((int)frameRateD) + "\n" + 
											std::to_string(*loadBalance1) + "," + std::to_string(*loadBalance2) + "," + std::to_string(*loadBalance3)
											+ "\n" + std::to_string(particles->ballV.size()) + "," + std::to_string(particles->ballAlive)
											+ "\n" + std::to_string(particles->bhV.size()) + "," + std::to_string(particles->bhAlive)
											+ "\n" + std::to_string((int)particles->maxParticleVel));
				mainWindow->draw(fps);
			}
						
			sfguiW.Display(*mainWindow);
			
			// Display drawn objects
			mainWindow->display();
		}
	}
		
	// Handle first portion of particles
	void calcPhysics1() {
		std::unique_lock<std::mutex> lock1(pauseMutex1);
				
		if (MULTITHREAD) {
			while(running){
				if (*threadsPaused) {
					pauseCV1.wait(lock1);
					clockP.restart();
				}
				
				*finishFlag1 = false;
				particles->quadSortParticles(0, *loadBalance1);
				*finishFlag1 = true;
				
				rendezvous1.wait();
				
				*finishFlag2 = false;
				particles->quadCollideParticles(0, *loadBalance2);
				*finishFlag2 = true;
								
				rendezvous2.wait();
				
				*finishFlag3 = false;
				particles->addPhysics(0, *loadBalance3);
				particles->cleanQuad();
				
				{ // Timekeeping
					elapsedTimeP = clockP.restart();
					
					tickTimeActual = TICKTIME_AVGFILT*elapsedTimeP.asSeconds() + tickTimeActual*(1.0 - TICKTIME_AVGFILT);
					tickTimeMax = std::min(Ball::diameterTable[DIA_SMALL]/(2.0*particles->maxParticleVel), MAX_TICKTIME);
					
					double tempScaleFactor =  std::min(tickTimeMax/tickTimeActual, scaleFactorM);
					if (scaleFactor > tempScaleFactor) scaleFactor = tempScaleFactor;
					else scaleFactor = SCALEFACT_AVGFILT*tickTimeActual*std::min(tickTimeMax/tickTimeActual, scaleFactorM)
														 + scaleFactor*(1.0 - tickTimeActual*SCALEFACT_AVGFILT);
					
					tickTime = tickTimeActual*scaleFactor;
					
					frameRateP = 1.0/tickTime;
				}
					
				*finishFlag3 = true;
				
				rendezvous3.wait();
			}
		}
		else {
			while(running){
				if (*threadsPaused) {
					pauseCV1.wait(lock1);
					clockP.restart();
				}
			
				particles->quadSortParticles(0, particles->size());
				particles->quadCollideParticles(0, particles->size());
				particles->cleanQuad();
				particles->addPhysics(0, particles->size());
				
				{ // Timekeeping
					elapsedTimeP = clockP.restart();
					
					tickTimeActual = TICKTIME_AVGFILT*elapsedTimeP.asSeconds() + tickTimeActual*(1.0 - TICKTIME_AVGFILT);
					tickTimeMax = std::min(DIA_SMALL/(2.0*particles->maxParticleVel), MAX_TICKTIME);
					
					double tempScaleFactor =  std::min(tickTimeMax/tickTimeActual, scaleFactorM);
					if (scaleFactor > tempScaleFactor) scaleFactor = tempScaleFactor;
					else scaleFactor = SCALEFACT_AVGFILT*tickTimeActual*std::min(tickTimeMax/tickTimeActual, scaleFactorM)
														 + scaleFactor*(1.0 - tickTimeActual*SCALEFACT_AVGFILT);
					
					tickTime = tickTimeActual*scaleFactor;
					
					frameRateP = 1.0/tickTime;
				}
			}
		}
	}
	
	// Handle second portion of particles
	void calcPhysics2() {
		std::unique_lock<std::mutex> lock2(pauseMutex2);
		while(running){
			if (*threadsPaused) {
				pauseCV2.wait(lock2);
			}
		
			particles->quadSortParticles(*loadBalance1, particles->size());
			if (*finishFlag1 == true) {
				 if (*loadBalance1 < particles->size() - 1) *loadBalance1 = *loadBalance1 + 1;
			}
			else if (*loadBalance1 > 0) *loadBalance1 = *loadBalance1 - 1;
			
			rendezvous1.wait();
			
			particles->quadCollideParticles(*loadBalance2, particles->size());
			if (*finishFlag2 == true) {
				 if (*loadBalance2 < particles->size() - 1) *loadBalance2 = *loadBalance2 + 1;
			}
			else if (*loadBalance2 > 0) *loadBalance2 = *loadBalance2 - 1;

			rendezvous2.wait();
			
			particles->addPhysics(*loadBalance3, particles->size());
			if (*finishFlag3 == true) {
				 if (*loadBalance3 < particles->size() - 1) *loadBalance3 = *loadBalance3 + 1;
			}
			else if (*loadBalance3 > 0) *loadBalance3 = *loadBalance3 - 1;
						
			rendezvous3.wait();
		}
	}

};
}

#endif