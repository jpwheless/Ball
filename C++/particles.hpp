#include "ball.hpp"
#include "blackHole.hpp"

namespace z {

class Particles {
public:
	bool particleCollisions;
	bool particleStickyness;
	
	std::vector<z::Ball> ballV;
	
	Particles() {}

	// Populate the window with balls in random locations
	void createInitBalls(int numBalls, float ballDia, float springRate,
												float rebEff, float attrRate, float attrRad) {
		// Create number of starting balls at random locations
		for ( int i = 0; i < numBalls; i++ ) {
			z::Ball ball;
			ball.setSize(ballDia);
			ball.setColor(rand()%255, rand()%255, rand()%255);
			ball.springRate = springRate;
			ball.reboundEfficiency = rebEff;
			ball.attrRad = attrRad;
			ball.attrRate = attrRate;

			
			float xPos, yPos;
			int tryCount = 0;
			bool collision = true;

			// Make sure ball doesn't collide with another upon start
			while (collision && tryCount <= 50) {
				xPos = ball.radius + (resX - 2.f*ball.radius)*rand()/(RAND_MAX + 1.0);
				yPos = ball.radius + (resY - 2.f*ball.radius)*rand()/(RAND_MAX + 1.0);
				collision = false;
				for (int j = 0; j < i; j++) {
					if (sqrt(pow(xPos - ballV[j].x, 2.0) + pow(yPos - ballV[j].y, 2.0)) < 2.f*ballDia)
						collision = true;
				}
				tryCount++;
			}
			
			ball.setPosition(xPos, yPos);
			
			ballV.push_back(ball);
		}
	}
	
	/////////////////////////
	// Thread Subfunctions //
	/////////////////////////
	void addPhysics(unsigned int iStart, unsigned int iStop) {
		for ( int i = iStart; i < iStop; i++ ) {
				ballV[i].update(tickTime, resX, resY, particleBoundary);
			}
	}

	void collisonUpdate(unsigned int iStart, unsigned int iStop) {
		
		for (int i = iStart; i < iStop; i++) {
			if (ballV[i].alive) {
				
				// Inter-particle collisions
				if (particleCollisions) {
					for (int j = i+1; j < ballV.size(); j++) {
						if (ballV[j].alive) {
						
							// Distance between the two points
							float dist = sqrt(pow(ballV[i].x - ballV[j].x, 2.0) + pow(ballV[i].y - ballV[j].y, 2.0));
							if (dist == 0) dist = 0.01; // Remove divide by zero errors
							
							float centerDist = ballV[i].radius + ballV[j].radius;
							
							// Check for particle collision
							if (dist < centerDist) { 
								float term = ((ballV[i].springRate + ballV[i].springRate)/2.f)
															*(centerDist - dist)*tickTime;
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
							else if (particleStickyness && dist < centerDist + std::max(ballV[i].attrRad, ballV[j].attrRad)) {
								
								// Figure out which attraction rates to use
								float attractRate = 0;
								if (dist < centerDist + ballV[i].attrRad) {
									attractRate += ballV[i].attrRate;
								}
								if (dist < centerDist + ballV[i].attrRad) {
									attractRate += ballV[j].attrRate;
								}
								
								float term = attractRate*tickTime/std::pow(dist, 2.f);
								float accel;
								
								accel = ((ballV[i].x - ballV[j].x)/dist)*term;
								ballV[i].xVel -= accel;
								ballV[j].xVel += accel;
								accel = ((ballV[i].y - ballV[j].y)/dist)*term;
								ballV[i].yVel -= accel;
								ballV[j].yVel += accel;
							
							}
						}
					}
				}
				
				// Particle-boundary collisions
				if (particleBoundary && ballV[i].x > resX - ballV[i].radius) {
					// Ball linear spring rate w/ wall rebound efficiency
					ballV[i].xVel += ((resX - ballV[i].radius) - ballV[i].x)*ballV[i].springRate*
					((ballV[i].xVel < 0) ? ballV[i].reboundEfficiency : 1.0)*tickTime;
				}
				else if (particleBoundary && ballV[i].x < ballV[i].radius) {         
					ballV[i].xVel += (ballV[i].radius - ballV[i].x)*ballV[i].springRate*
					((ballV[i].xVel > 0) ? ballV[i].reboundEfficiency : 1.0)*tickTime;
				}

				if (particleBoundary && ballV[i].y > resY - ballV[i].radius) {
					ballV[i].yVel += ((resY - ballV[i].radius) - ballV[i].y)*ballV[i].springRate*
					((ballV[i].yVel < 0) ? ballV[i].reboundEfficiency : 1.0)*tickTime;
				}
				else if (particleBoundary && ballV[i].y < ballV[i].radius) {
					ballV[i].yVel += (ballV[i].radius - ballV[i].y)*ballV[i].springRate*
					((ballV[i].yVel > 0) ? ballV[i].reboundEfficiency : 1.0)*tickTime;
				}
				else {
					// Linear gravity
					ballV[i].yVel += linGravity*tickTime;
				}
			}
		}
	}
	
	draw(sf::RenderWindow* mainWindow) {
		// Draw all particles in ball vector
		for ( int i = 0; i < ballV.size(); i++ ) {
			mainWindow->draw(ballV[i].ballShape);
		}
	}
};

}