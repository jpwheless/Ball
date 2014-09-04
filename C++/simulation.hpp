#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <windows.h>

#include "ball.hpp"
#include "vector.hpp"
#include "blackHole.hpp"

namespace gp {

class Simulation {
public:
	int resX, resY;
	float xGravity, yGravity;

	int numberOfBalls;
	std::vector<gp::Ball> ballV;

	sf::RenderWindow* window;
	//window.setFramerateLimit(60);
	sf::Clock clock;
	sf::Time elapsedTime;
	float tickTime, frameRateAvg;
	//sf::Texture bgTexture;
	//sf::Sprite bg;
	std::string windowTitle;
	//sf::Texture ballTexture;
	int ballDia;

	bool running;

	// Threads
	std::thread* drawThread;
	std::thread* computePhysicsThread;

	Simulation() {

		//////////////
		// Settings //
		//////////////
		
		// Resolution
		resX = 1500; // n.5*ballDia for even particle stacking
		resY = 800;
		windowTitle = "Balls! ";
		// Gravity
		xGravity = 0.0;
		yGravity = 1000.0;
		// Balls
		numberOfBalls = 1000;
		// Ball Size
		ballDia = 10; // Scales texture if necessary
		


		////////////////////
		// Initialization //
		////////////////////
		//bgTexture.loadFromFile( "textures/bg.png" );
		//bg.setTexture(bgTexture);
		//ballTexture.loadFromFile("textures/ball.png");
		tickTime = 0;
		frameRateAvg = 0;
		// Create ball data structure
		for ( int i = 0; i < numberOfBalls; i++ ) {
			gp::Ball ball;
			ball.setSize(ballDia, rand()%255, rand()%255, rand()%255);
			ball.springRate = 50000;
			ball.reboundEfficiency = 0.7;

			
			float xPos, yPos;
			int tryCount = 1;
			boolean collision = true;

			// Make sure ball doesn't collide with another upon start
			while (collision && tryCount <= 50) {
				xPos = ball.radius + (resX - 2.f*ball.radius)*rand()/(RAND_MAX + 1.0);
				yPos = ball.radius + (resY - 2.f*ball.radius)*rand()/(RAND_MAX + 1.0);
				collision = false;
				for (int j = 0; j < i; j++) {
					if (sqrt(pow(xPos - ballV[j].x, 2.0) + pow(yPos - ballV[j].y, 2.0)) < ballDia)
						collision = true;
				}
				tryCount++;
			}
			
			ball.setPosition(xPos, yPos);
			//ball.setTexture(ballTexture, ballDia, rand()%255, rand()%255, rand()%255);
			
			ballV.push_back(ball);
		}
	}

	~Simulation() {
		// Clean up
		delete window;
		delete drawThread;
		delete computePhysicsThread;
	}

	///////////////////
	// Start Threads //
	///////////////////
	void launch() {
		running = true;
		drawThread = new std::thread(&Simulation::draw, this);
		computePhysicsThread = new std::thread(&Simulation::computePhysics, this);
		drawThread->join();
		computePhysicsThread->join();
	}

	void draw() {
		window = new sf::RenderWindow( sf::VideoMode( resX, resY ), 
			windowTitle + std::to_string((int)frameRateAvg), sf::Style::Default,
			sf::ContextSettings( 24, 8, 2, 3, 0));
		window->setFramerateLimit(70);
		while (window->isOpen()) {
			sf::Event event;
			while (window->pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window->close();
					running = false;
				}
			}

			window->clear();
			//window->draw(bg);
			for ( int i = 0; i < ballV.size(); i++ ) {
				window->draw(ballV[i].ballShape);
			}
			window->display();
			
			window->setTitle(windowTitle + std::to_string((int)frameRateAvg));
		}
	}

	void computePhysics() {
		Sleep(100);
		while(running){
			collisonUpdate();
			for ( int i = 0; i < ballV.size(); i++ ) {
				ballV[i].update( tickTime );
			}
			
			elapsedTime = clock.restart();
			tickTime = 0.01f*elapsedTime.asSeconds() + tickTime*(1.f - 0.01f);
			//frameRateAvg = 0.01f/tickTime + frameRateAvg*(1.f - 0.01f);
			frameRateAvg = 1.f/tickTime;
		}
	}

	void collisonUpdate() {
		
		for (int i = 0; i < ballV.size() - 1; i++) {
			if (ballV[i].alive) {
				
				// Inter-particle collisions
				for (int j = i+1; j < ballV.size(); j++) {
					if (ballV[j].alive) {
						// Distance between the two points
						float dist = sqrt(pow(ballV[i].x - ballV[j].x, 2.0) + pow(ballV[i].y - ballV[j].y, 2.0));
						if (dist < ballV[i].radius + ballV[j].radius) { // Particles are colliding
							float term = ballV[i].springRate*(ballV[i].radius + ballV[j].radius - dist)*tickTime;
							float accel;
							
							accel = ((ballV[i].x - ballV[j].x)/dist)*term*
								(((ballV[i].x < ballV[j].x && ballV[i].xVel < ballV[j].xVel) ||
								(ballV[i].x > ballV[j].x && ballV[i].xVel > ballV[j].xVel)) ? ballV[i].reboundEfficiency : 1.0);
							ballV[i].xVel += accel;
							ballV[j].xVel -= accel;
							accel = ((ballV[i].y - ballV[j].y)/dist)*term*
								(((ballV[i].y < ballV[j].y && ballV[i].yVel < ballV[j].yVel) ||
								(ballV[i].y > ballV[j].y && ballV[i].yVel > ballV[j].yVel)) ? ballV[i].reboundEfficiency : 1.0);
							ballV[i].yVel += accel;
							ballV[j].yVel -= accel;
						}
					}
				}
				
				// Particle-boundary collisions
				if (ballV[i].x > resX - ballV[i].radius) {
					// Ball linear spring rate w/ wall rebound efficiency
					ballV[i].xVel += ((resX - ballV[i].radius) - ballV[i].x)*ballV[i].springRate*
					((ballV[i].xVel < 0) ? ballV[i].reboundEfficiency : 1.0)*tickTime;
				}
				else if (ballV[i].x < ballV[i].radius) {         
					ballV[i].xVel += (ballV[i].radius - ballV[i].x)*ballV[i].springRate*
					((ballV[i].xVel > 0) ? ballV[i].reboundEfficiency : 1.0)*tickTime;
				}
				else {
					ballV[i].xVel += xGravity*tickTime;
				}

				if (ballV[i].y > resY - ballV[i].radius) {
					ballV[i].yVel += ((resY - ballV[i].radius) - ballV[i].y)*ballV[i].springRate*
					((ballV[i].yVel < 0) ? ballV[i].reboundEfficiency : 1.0)*tickTime;
				}
				else if (ballV[i].y < ballV[i].radius) {
					ballV[i].yVel += (ballV[i].radius - ballV[i].y)*ballV[i].springRate*
					((ballV[i].yVel > 0) ? ballV[i].reboundEfficiency : 1.0)*tickTime;
				}
				else {
					ballV[i].yVel += yGravity*tickTime;
				}
			}
		}
	}

};

}

#endif