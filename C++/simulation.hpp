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
	float wallRebEff, wallSpringRate;
	bool enableLinGrav;

	int numberOfBalls;
	std::vector<gp::Ball> balls;

	sf::RenderWindow* window;
	//window.setFramerateLimit(60);
	sf::Clock timer;
	sf::Time elapsed;
	float tickTime;
	sf::Texture bgTexture;
	sf::Sprite bg;
	std::string windowTitle;
	sf::Texture ballTexture;
	int imageSize;

	bool running;

	// Threads
	std::thread* drawThread;
	std::thread* computePhysicsThread;

	Simulation() {

		//////////////
		// Settings //
		//////////////
		
		// Resolution
		resX = 1200;
		resY = 720;
		windowTitle = "Balls!";
		// Gravity
		enableLinGrav = true;
		xGravity = 0;
		yGravity = 500;
		// Balls
		numberOfBalls = 200;
		// Walls
		wallRebEff = 0.01;
		wallSpringRate = 10000;
		// Ball Image Size
		imageSize = 10;


		////////////////////
		// Initialization //
		////////////////////
		bgTexture.loadFromFile( "textures/bg.png" );
		bg.setTexture(bgTexture);
		ballTexture.loadFromFile("textures/ball.png");
		tickTime = 0;
		// Create ball data structure
		for ( int i = 0; i < numberOfBalls; i++ ) {
			gp::Ball ball;
			ball.setPosition(rand()%resX, rand()%resY);
			ball.springRate = 5000;
			ball.sprite.setOrigin(ball.radius, ball.radius);
			ball.setTexture(ballTexture, imageSize);
			balls.push_back(ball);
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
		window = new sf::RenderWindow( sf::VideoMode( resX, resY ), windowTitle);
		window->setFramerateLimit(70);
		while (window->isOpen())
		{
			sf::Event event;
			while (window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed){
					window->close();
					running = false;
				}
			}

			window->clear();
			window->draw(bg);
			for ( int i = 0; i < balls.size(); i++ ){
				window->draw(balls[i].sprite);
			}
			window->display();
		}
	}

	void computePhysics() {
		Sleep(100);
		while(running){
			collisonUpdate();
			applyGravity();
			for ( int i = 0; i < balls.size(); i++ )
				balls[i].update( tickTime );

			elapsed = timer.restart();
			tickTime = elapsed.asSeconds();
		}
	}

	void collisonUpdate() {
		for (int i = 0; i < balls.size() - 1; i++) {
			if (balls[i].alive) {
				float ballSpringRate = balls[i].springRate;
				for (int j = i+1; j < balls.size(); j++) {
				// Distance between the two points
					if (balls[j].alive) {
						float rad = sqrt(pow(balls[i].x - balls[j].x, 2) + pow(balls[i].y - balls[j].y, 2));
						if (rad < balls[i].radius + balls[j].radius) { // Particles are colliding
							float term = (ballSpringRate*(balls[i].radius)*(tickTime));

							balls[j].xVel += ((balls[j].x - balls[i].x)/rad)*term;
							balls[i].xVel += ((balls[i].x - balls[j].x)/rad)*term;
							balls[j].yVel += ((balls[j].y - balls[i].y)/rad)*term;
							balls[i].yVel += ((balls[i].y - balls[j].y)/rad)*term;
						}
					}
				}
			}
		}
	}

	void applyGravity() {
		float border = imageSize/2;
		for (int i = 0; i < balls.size(); i++) {
			if (balls[i].alive) {
				if (balls[i].x > resX - border) {
					// Ball linear spring rate w/ wall rebound efficiency
					balls[i].xVel += ((resX-border) - balls[i].x)*wallSpringRate*
					((balls[i].xVel < 0) ? wallRebEff : 1.0)*tickTime;
				}
				else if (balls[i].x < border) {         
					balls[i].xVel += (border - balls[i].x)*wallSpringRate*
					((balls[i].xVel > 0) ? wallRebEff : 1.0)*tickTime;
				}
				else {
					if (enableLinGrav) balls[i].xVel += xGravity*tickTime;
				}

				if (balls[i].y > resY - border) {
					// Ball linear spring rate w/ wall rebound efficiency
					balls[i].yVel += ((resY-border) - balls[i].y)*wallSpringRate*
					((balls[i].yVel < 0) ? wallRebEff : 1.0)*tickTime;
				}
				else if (balls[i].y < border) {
					balls[i].yVel += (border - balls[i].y)*wallSpringRate*
					((balls[i].yVel > 0) ? wallRebEff : 1.0)*tickTime;
				}
				else {
					if (enableLinGrav) balls[i].yVel += yGravity*tickTime;
				}
			}
		}
	}
};

}

#endif