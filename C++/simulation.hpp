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
#include <iostream>
#include <thread>
#include <windows.h>

#include "barrier.hpp"
#include "particles.hpp"
#include "input.hpp"

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
	sfg::CheckButton::Ptr cbBoundaries;
	sfg::Button::Ptr bDebug;
	sfg::Button::Ptr bClean;
	
	sfg::RadioButton::Ptr mouseFuncRadio1; // Erase
	sfg::RadioButton::Ptr mouseFuncRadio2; // Drag
	sfg::RadioButton::Ptr mouseFuncRadio3; // Paint
	sfg::RadioButton::Ptr mouseFuncRadio4; // Shoot
	sfg::RadioButton::Ptr mouseFuncRadio5; // Blackhole

	sf::Clock clock;
	sf::Time elapsedTime;
	double tickTime, tickTimeA;
	double scaleFactor, frameRateAvg;
	
	bool running;
	bool debugRead;
	
	// Threads
	std::thread* drawThread;
	std::thread* calcPhysicsThread1;
	std::thread* calcPhysicsThread2;
	
	z::SpinningBarrier rendezvous1 = z::SpinningBarrier(2);
	z::SpinningBarrier rendezvous2 = z::SpinningBarrier(2);
	
	std::atomic<int>* loadBalance1 = new std::atomic<int>;
	std::atomic<int>* loadBalance2 = new std::atomic<int>;
	std::atomic<bool>* finishFlag1 = new std::atomic<bool>;
	std::atomic<bool>* finishFlag2 = new std::atomic<bool>;
	
	// SFGUI button functions
	void buttonCollision() {
		particles->particleCollisions = cbCollision->IsActive();
	}
	void buttonStickyness() {
		particles->particleStickyness = cbStickyness->IsActive();
	}
	void buttonBoundaries() {
		particles->particleBoundary = cbBoundaries->IsActive();
	}
	void buttonDebug() {
		for (int i = 0; i < particles->ballV.size(); i++) {
			std::cout << "Particle " << i << ": ";
			if (particles->ballV[i].alive) {
				std::cout << "Vel = " << sqrt(pow(particles->ballV[i].xVel, 2.f) + pow(particles->ballV[i].yVel, 2.f));
				std::cout <<", x = " << particles->ballV[i].x << ", y = " << particles->ballV[i].y << "\n";
			}
			else {
				std::cout << "Inactive\n";
			}
		}
		std::cout << "\n";
	}
	void buttonClean() {
		particles->cleanParticles();
	}
	void buttonMouseSelect () {
		if(mouseFuncRadio1->IsActive()) input->mouseMode = 1;
		else if(mouseFuncRadio2->IsActive()) input->mouseMode = 2;
		else if(mouseFuncRadio3->IsActive()) input->mouseMode = 3;
		else if(mouseFuncRadio4->IsActive()) input->mouseMode = 4;
		else if(mouseFuncRadio5->IsActive()) input->mouseMode = 5;
	}
	
	// Call after initGUI().
	void initSFML() {
		sf::Vector2f requisition = guiWindow->GetRequisition();
		mainWindow = new sf::RenderWindow( sf::VideoMode(resX + requisition.x, resY), 
			"Particles!", sf::Style::Default,
			sf::ContextSettings( 24, 8, 8, 3, 0)); // Set openGL parameters
		mainWindow->setFramerateLimit(120);
		
		input = new z::Input(particles, mainWindow);
		
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
	}

	void initGUI() {
		guiWindow = sfg::Window::Create();
		guiWindow->SetStyle(guiWindow->GetStyle() ^ sfg::Window::TITLEBAR);
		guiWindow->SetStyle(guiWindow->GetStyle() ^ sfg::Window::RESIZE);
		auto boxMain = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
		auto boxSim = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
		auto boxMouse = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
		
		auto label1 = sfg::Label::Create();
		auto label2 = sfg::Label::Create();
		label1->SetText("Sim Parameters");
		label2->SetText("Mouse Function");
		
		auto separatorh = sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL);
		
		// Button functions
		cbCollision = sfg::CheckButton::Create("Collision");
		cbCollision->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonCollision, this));
		cbCollision->SetActive(particles->particleCollisions);
		
		cbStickyness = sfg::CheckButton::Create("Stickyness");
		cbStickyness->GetSignal( sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonStickyness, this));
		cbStickyness->SetActive(particles->particleStickyness);
		
		cbBoundaries = sfg::CheckButton::Create("Boundaries");
		cbBoundaries->GetSignal( sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonBoundaries, this));
		cbBoundaries->SetActive(particles->particleBoundary);

		bDebug = sfg::Button::Create("Debug");
		bDebug->GetSignal( sfg::Widget::OnLeftClick).Connect(std::bind(&z::Simulation::buttonDebug, this));
		
		bClean = sfg::Button::Create("Clean");
		bClean->GetSignal( sfg::Widget::OnLeftClick).Connect(std::bind(&z::Simulation::buttonClean, this));
		
		mouseFuncRadio1 = sfg::RadioButton::Create("Erase");
		mouseFuncRadio2 = sfg::RadioButton::Create("Drag", mouseFuncRadio1->GetGroup());
		mouseFuncRadio3 = sfg::RadioButton::Create("Paint", mouseFuncRadio1->GetGroup());
		mouseFuncRadio4 = sfg::RadioButton::Create("Shoot", mouseFuncRadio1->GetGroup());
		mouseFuncRadio5 = sfg::RadioButton::Create("Blackhole", mouseFuncRadio1->GetGroup());
		mouseFuncRadio1->SetActive(true);
		mouseFuncRadio1->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		mouseFuncRadio2->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		mouseFuncRadio3->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		mouseFuncRadio4->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));
		mouseFuncRadio5->GetSignal(sfg::ToggleButton::OnToggle).Connect(std::bind(&z::Simulation::buttonMouseSelect, this));

		boxSim->Pack(bDebug);
		boxSim->Pack(bClean);
		boxSim->Pack(label1);
		boxSim->Pack(cbCollision);
		boxSim->Pack(cbStickyness);
		boxSim->Pack(cbBoundaries);
		boxMouse->Pack(label2);
		boxMouse->Pack(mouseFuncRadio1);
		boxMouse->Pack(mouseFuncRadio2);
		boxMouse->Pack(mouseFuncRadio3);
		boxMouse->Pack(mouseFuncRadio4);
		boxMouse->Pack(mouseFuncRadio5);
		boxMain->Pack(boxSim);
		boxMain->Pack(separatorh);
		boxMain->Pack(boxMouse);
		
		guiWindow->Add(boxMain);
		
		guiWindow->SetPosition(sf::Vector2f(resX, 0));
	}
		
public:

	int resX, resY;
	double linGravity;
	sf::RenderWindow* mainWindow;
	z::Input *input;
	
	z::Particles *particles = new Particles(&resX, &resY, &tickTime, &linGravity);

	////////////////////
	// Initialization //
	////////////////////
	Simulation() {
		srand(static_cast<unsigned>(time(0)));
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
		resX = 1500;
		resY = 900;
		linGravity = 1000.0;
		
			// Particle Defaults
		particles->initialNumBalls = 500;
		particles->defaultBallDia = 10.0;
		particles->defaultBallSprRate = 50000.f;
		particles->defaultBallebEff = 0.9;
		particles->defaultBallAttrRate = 25000.f * pow(particles->defaultBallDia/2.f, 2.f); // Surface rate to center rate
		particles->defaultBallAttrRad = 5.f;	

		particles->particleCollisions = true;
		particles->particleStickyness = true;
		particles->particleBoundary = true;
		
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
							if (temp >= 1.f) particles->defaultBallDia = temp;
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
					}				else if (paramName == "PARTICLE_COLLISIONS") {
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
			createParameter << "PARTICLE_BOUNDARY=" << ((particles->particleBoundary) ? "true" : "false") << "\n";
			createParameter << "PARTICLE_COLLISIONS=" << ((particles->particleCollisions) ? "true" : "false") << "\n";
			createParameter << "PARTICLE_STICKYNESS=" << ((particles->particleStickyness) ? "true" : "false") << "\n";
		}
		createParameter.close();
		
		particles->createInitBalls();
	}
		
	void launch() {
		running = true;
		drawThread = new std::thread(&Simulation::draw, this);
		calcPhysicsThread1 = new std::thread(&Simulation::calcPhysics1, this);
		calcPhysicsThread2 = new std::thread(&Simulation::calcPhysics2, this);
		
		*loadBalance1 = particles->ballV.size() / 3.f;
		*loadBalance2 = particles->ballV.size() / 2.f;
		
		tickTime = 0.002; // Jump start to avoid physics glitches
		frameRateAvg = 500;
						
		drawThread->join();
		calcPhysicsThread1->join();
		calcPhysicsThread2->join();
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
			
			// Handle events
			sf::Event event;
			while (mainWindow->pollEvent(event)) {
				guiWindow->HandleEvent(event);
				switch(event.type) {
					case sf::Event::Closed:
						mainWindow->close();
						running = false;
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
			guiWindow->Update(1.f);
			mainWindow->clear();
			
			
			particles->draw(mainWindow);
			input->draw();
			mainWindow->draw(menuDivider, 2, sf::Lines);
			
			// Draw text/gui
			if (debugRead) {
				std::string temp = std::to_string(scaleFactor);
				temp.resize(4);
				fps.setString(std::to_string((int)frameRateAvg) + "," + temp + "\n" + 
											std::to_string(*loadBalance1) + "," + std::to_string(*loadBalance2)
											+ "\n" + std::to_string(particles->ballV.size()) + "," + std::to_string(particles->numAlive));
				mainWindow->draw(fps);
			}
			sfguiW.Display(*mainWindow);
			
			// Display drawn objects
			mainWindow->display();
		}
	}
		
	// Handle first portion of particles
	void calcPhysics1() {

		while(running){
		
			*finishFlag1 = false;
			particles->collisonUpdate(0, *loadBalance1);
			*finishFlag1 = true;
			
			rendezvous1.wait();
			
			*finishFlag2 = false;
			particles->addPhysics(0, *loadBalance2);
			*finishFlag2 = true;
			
			// Timekeeping
			elapsedTime = clock.restart();
			tickTimeA = 0.01f*elapsedTime.asSeconds() + tickTimeA*(1.f - 0.01f);
			if (tickTimeA <= 1.f/400.f) {
				scaleFactor = 1.0;
				tickTime = tickTimeA;
			}
			else {
				scaleFactor = (1.f/400.f)/tickTimeA;
				tickTime = tickTimeA*scaleFactor;
			}
			frameRateAvg = 1.f/tickTime;
			
			rendezvous2.wait();
		}
	}
	
	// Handle second portion of particles
	void calcPhysics2() {
		while(running){
			particles->collisonUpdate(*loadBalance1, particles->size());
			
			// If thread #1 has finished first, then increase its load
			if (*finishFlag1 == true) *loadBalance1 = *loadBalance1 + 1;
			else *loadBalance1 = *loadBalance1 - 1;
			if (*loadBalance1 < 0) *loadBalance1 = 0;
			else if (*loadBalance1 > particles->size()) *loadBalance1 = particles->size();
			
			rendezvous1.wait();
			particles->addPhysics(*loadBalance2, particles->size());
			
			if (*finishFlag2 == true) *loadBalance2 = *loadBalance2 + 1;
			else *loadBalance2 = *loadBalance2 - 1;
			if (*loadBalance2 < 0) *loadBalance2 = 0;
			else if (*loadBalance2 > particles->size()) *loadBalance2 = particles->size();
						
			rendezvous2.wait();
		}
	}

};

}

#endif