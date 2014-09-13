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

namespace z {

class Simulation {
private:
	sf::RenderWindow* mainWindow;
	sf::Font calibri;
	sf::Text fps;
	
	// SFGUI
	sfg::SFGUI sfguiW;
	sfg::Window::Ptr guiWindow;
	sfg::CheckButton::Ptr cbStickyness;
	sfg::CheckButton::Ptr cbCollision;
	sfg::CheckButton::Ptr cbBoundaries;

	sf::Clock clock;
	sf::Time elapsedTime;
	float tickTime, frameRateAvg;
	
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
		particleCollisions = cbCollision->IsActive();
	}
	void buttonStickyness() {
		particleStickyness = cbStickyness->IsActive();
	}
	void buttonBoundaries() {
		particleBoundary = cbBoundaries->IsActive();
	}
	
	void initSFML() {
		mainWindow = new sf::RenderWindow( sf::VideoMode( resX, resY ), 
			"Particles!", sf::Style::Default,
			sf::ContextSettings( 24, 8, 8, 3, 0)); // Set openGL parameters
		mainWindow->setFramerateLimit(60);
		
		// fps readout
		if(calibri.loadFromFile("%windir%\fonts\calibri.ttf")) {
			debugRead = true;
			fps.setFont(calibri);
			fps.setCharacterSize(15);
			fps.setColor(sf::Color::White);
		}
	}

	void initGUI() {
		guiWindow = sfg::Window::Create();
		guiWindow->SetTitle( "Title" );
		auto box = sfg::Box::Create( sfg::Box::Orientation::VERTICAL, 5.f );
		guiWindow->Add(box);
		
		// Button functions
		cbCollision = sfg::CheckButton::Create( "Collision" );
		cbCollision->GetSignal( sfg::ToggleButton::OnToggle ).Connect( std::bind( &z::Simulation::buttonCollision, this ) );
		cbCollision->SetActive(particleCollisions);
		
		cbStickyness = sfg::CheckButton::Create( "Stickyness" );
		cbStickyness->GetSignal( sfg::ToggleButton::OnToggle ).Connect( std::bind( &z::Simulation::buttonStickyness, this ) );
		cbStickyness->SetActive(particleStickyness);
		
		cbBoundaries = sfg::CheckButton::Create( "Boundaries" );
		cbBoundaries->GetSignal( sfg::ToggleButton::OnToggle ).Connect( std::bind( &z::Simulation::buttonBoundaries, this ) );
		cbBoundaries->SetActive(particleBoundary);

		box->Pack(cbCollision);
		box->Pack(cbStickyness);
		box->Pack(cbBoundaries);
	}
		
public:

	int resX, resY;
	float linGravity;
	bool particleBoundary;
	
	z::Particles particles;

	////////////////////
	// Initialization //
	////////////////////
	Simulation() {

		loadParams();
						
	}

	~Simulation() {
		// Clean up
		delete mainWindow;
		delete drawThread;
		delete calcPhysicsThread1;
		delete calcPhysicsThread2;
	}
	
	void loadParams() {
		// Default simulation parameters
		resX = 1500;
		resY = 900;
		linGravity = 1000.0;
		
		int initialNumBalls = 500;
		float initialBallDia = 10.0;
		float initBallSprRate = 50000.f;
		float initBallebEff = 0.7;
		float initBallAttrRate = 25000.f * pow(initialBallDia/2.f, 2.f); // Surface rate to center rate
		float initBallAttrRad = 5.f;
		
		bool particleCollisions = true;
		bool particleStickyness = true;
		particleBoundary = true;
		
		// Open file and read values
		ifstream parameterFile("parameters.ini");
		if (parameterFile.is_open()) { // Read parameter file
			
			string line;
			
			// Read all lines of file, sorting values into variables
			while(getline(parameterFile, line)) {
				istringstream lineSS(line);
				
				string paramName;
				if (getline(lineSS, paramName, '=')) {
					if (paramName == "WINDOW_WIDTH") {
						string paramNum;
						if (getline(lineSS, paramNum, ' ')) {
							int temp = stoi(paramNum);
							if (temp >= 100) resX = temp;
						}
					}
					else if (paramName == "WINDOW_HEIGHT") {
						string paramNum;
						if (getline(lineSS, paramNum, ' ')) {
							int temp = stoi(paramNum);
							if (temp >= 100) resY = temp;
						}
					}
					else if (paramName == "LINEAR_GRAVITY") {
						string paramNum;
						if (getline(lineSS, paramNum, ' ')) {
							linGravity = stof(paramNum);
						}
					}
					else if (paramName == "INITIAL_PARTICLES") {
						string paramNum;
						if (getline(lineSS, paramNum, ' ')) {
							int temp = stoi(paramNum);
							if (temp >= 0) initialNumBalls = temp;
						}
					}
					else if (paramName == "INITIAL_PARTICLE_DIAMETER") {
						string paramNum;
						if (getline(lineSS, paramNum, ' ')) {
							float temp = stof(paramNum);
							if (temp >= 1.f) initialBallDia = temp;
						}
					}
					else if (paramName == "INITIAL_PARTICLE_ATTRACT_RATE") {
						string paramNum;
						if (getline(lineSS, paramNum, ' ')) {
							initBallAttrRate = stof(paramNum)*pow(initialBallDia/2.f, 2.f);;
						}
					}
					else if (paramName == "INITIAL_PARTICLE_ATTRACT_RADIUS") {
						string paramNum;
						if (getline(lineSS, paramNum, ' ')) {
							float temp = stof(paramNum);
							if (temp >= 0) initBallAttrRad = temp;
						}
					}
					else if (paramName == "PARTICLE_BOUNDARY") {
						string paramBool;
						if (getline(lineSS, paramBool, ' ')) {
							if (paramBool == "true") particleBoundary = true;
							else if (paramBool == "false") particleBoundary = false;
						}
					}				else if (paramName == "PARTICLE_COLLISIONS") {
						string paramBool;
						if (getline(lineSS, paramBool, ' ')) {
							if (paramBool == "true") particleCollisions = true;
							else if (paramBool == "false") particleCollisions = false;
						}
					}
					else if (paramName == "PARTICLE_STICKYNESS") {
						string paramBool;
						if (getline(lineSS, paramBool, ' ')) {
							if (paramBool == "true") particleStickyness = true;
							else if (paramBool == "false") particleStickyness = false;
						}
					}
				}
			}
		}
		parameterFile.close();
		
		// Assume reformatting/creation is necessary
		ofstream createParameter("parameters.ini");
		if (createParameter.is_open()) {
			createParameter << "WINDOW_WIDTH=" << resX << "\n";
			createParameter << "WINDOW_HEIGHT=" << resY << "\n";
			createParameter << "LINEAR_GRAVITY=" << linGravity << "\n";
			createParameter << "INITIAL_PARTICLES=" << initialNumBalls << "\n";
			createParameter << "INITIAL_PARTICLE_DIAMETER=" << initialBallDia << "\n";
			createParameter << "INITIAL_PARTICLE_ATTRACT_RATE=" << initBallAttrRate/pow(initialBallDia/2.f, 2.f) << "\n";
			createParameter << "INITIAL_PARTICLE_ATTRACT_RADIUS=" << initBallAttrRad << "\n";
			createParameter << "PARTICLE_BOUNDARY=" << ((particleBoundary) ? "true" : "false") << "\n";
			createParameter << "PARTICLE_COLLISIONS=" << ((particleCollisions) ? "true" : "false") << "\n";
			createParameter << "PARTICLE_STICKYNESS=" << ((particleStickyness) ? "true" : "false") << "\n";
		}
		createParameter.close();
		
		particles.createInitBalls(initialNumBalls, initialBallDia, initBallSprRate,
															initBallebEff, initBallAttrRate, initBallAttrRad);
		particles.particleCollisions = particleCollisions;
		particles.particleStickyness = particleStickyness;
	}
		
	void launch() {
		running = true;
		drawThread = new std::thread(&Simulation::draw, this);
		calcPhysicsThread1 = new std::thread(&Simulation::calcPhysics1, this);
		calcPhysicsThread2 = new std::thread(&Simulation::calcPhysics2, this);
		
		*loadBalance1 = particles.ballV.size() / 3.f;
		*loadBalance2 = particles.ballV.size() / 2.f;
		
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
		initSFML();		
		initGUI();

		while (mainWindow->isOpen()) {
			sf::Event event;
			while (mainWindow->pollEvent(event)) {
				guiWindow->HandleEvent(event);
				if (event.type == sf::Event::Closed) {
					mainWindow->close();
					running = false;
				}
			}
			
			guiWindow->Update(1.f);
			mainWindow->clear();
			
			particles.draw(mainWindow);
			
			// Draw text/gui
			if (debugRead) {
				fps.setString(std::to_string((int)frameRateAvg) + "\n" + 
											std::to_string(*loadBalance1) + "," + std::to_string(*loadBalance2)
											+ "\n" + std::to_string(particles.ballV.size()));
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
			particles.collisonUpdate(0, *loadBalance1);
			*finishFlag1 = true;
			
			rendezvous1.wait();
			
			*finishFlag2 = false;
			particles.addPhysics(0, *loadBalance2);
			*finishFlag2 = true;

			elapsedTime = clock.restart();
			tickTime = 0.01f*elapsedTime.asSeconds() + tickTime*(1.f - 0.01f);
			
			frameRateAvg = 1.f/tickTime;
			
			rendezvous2.wait();
		}
	}
	
	// Handle second portion of particles
	void calcPhysics2() {
		while(running){
			particles.collisonUpdate(*loadBalance1, ballV.size());
			
			// If thread #1 has finished first, then increase its load
			if (*finishFlag1 == true) *loadBalance1 = *loadBalance1 + 1;
			else *loadBalance1 = *loadBalance1 - 1;
			if (*loadBalance1 < 0) *loadBalance1 = 0;
			else if (*loadBalance1 > ballV.size()) *loadBalance1 = ballV.size();
			
			rendezvous1.wait();
			particles.addPhysics(*loadBalance2, ballV.size());
			
			if (*finishFlag2 == true) *loadBalance2 = *loadBalance2 + 1;
			else *loadBalance2 = *loadBalance2 - 1;
			if (*loadBalance2 < 0) *loadBalance2 = 0;
			else if (*loadBalance2 > ballV.size()) *loadBalance2 = ballV.size();
						
			rendezvous2.wait();
		}
	}

};

}

#endif