#include <cmath>

#include "ball.hpp"
#include "blackHole.hpp"
#include "quad.hpp"

#define NUM_TRIES 25
#define PI 3.14159265359
#define PI2 6.28318530718
#define PI60 1.04719755

#define MAX_PARTICLES 10000
#define MAX_BH 1000
#define PARTICLE_CLEAN 500
#define BH_CLEAN 10


namespace z {

class Particles {
private:
	Quad* quadTree;

public:
	int *resX, *resY;
	double *tickTime;
	double linGravity;
	bool particleCollisions;
	bool particleStickyness;
	bool boundCeiling;
	bool boundWalls;
	bool boundFloor;
		
	double prevX;
	double prevY;
	
	std::vector<z::Ball*> ballV;
	std::vector<z::BlackHole> bhV;
	
	std::vector<int> listParticles;
	std::vector<int> listBH;
	
	int ballAlive;
	int bhAlive;
	
	double maxParticleVel;
	
	////////////////////
	// Constructor //
	////////////////////
	
	Particles(int *resXT, int *resYT, double *tickTimeT, double linGravityT) {
		resX = resXT;
		resY = resYT;
		tickTime = tickTimeT;
		linGravity = linGravityT;
		
		quadTree = new Quad(NULL, 0, 4, 0, 0, *resX, 0, *resY);
		
		BlackHole::tickTime = tickTime;
		
		Ball::boundCeiling = &boundCeiling;
		Ball::boundWalls = &boundWalls;
		Ball::boundFloor = &boundFloor;
		Ball::sticky = &particleStickyness;
		Ball::tickTime = tickTimeT;
		Ball::resX = resXT;
		Ball::resY = resYT;

		ballV.reserve(MAX_PARTICLES);
		bhV.reserve(MAX_BH);
						
		z::BlackHole bhPerm = BlackHole(*resX/2.f, *resY/2.f, 0, 20, COLLISION);
		bhV.push_back(bhPerm);
		bhAlive = 1;
		ballAlive = 0;
		maxParticleVel = 0;
	}
	
	~Particles() {
		for (int k = 0; k < ballV.size(); k++) delete ballV[k];
		delete quadTree;
	}
		
	double randDouble(double minimum, double maximum) {
		double r = (double)rand()/(double)RAND_MAX;
		return minimum + r*(maximum - minimum);
	}
	
	///////////////////////
	// Particle Creation //
	///////////////////////
	
	// Populate the window with balls in random locations
	void createInitBalls(int numBalls, double ballDia, double ballDensity, double springRate,
											double rebEff, double attrRate, double attrRad) {
		// Create number of starting balls at random locations
		for ( int i = 0; i < numBalls; i++ ) {
			z::Ball *ball;
			ball = new Ball();
			ball->setSize(ballDia);
			ball->setMass(ballDensity);
			ball->setColor(rand()%255, rand()%255, rand()%255);
			ball->springRate = springRate;
			ball->reboundEfficiency = rebEff;
			ball->setSticky(attrRad, attrRate);
			
			double xPos, yPos;
			int tryCount = 0;
			bool collision = true;

			// Make sure ball doesn't collide with another upon start
			while (collision && tryCount <= NUM_TRIES) {
				xPos = randDouble(ball->radius, *resX - ball->radius);
				yPos = randDouble(ball->radius, *resY - ball->radius);
				collision = false;
				for (int j = 0; j < i; j++) {
					if (sqrt(pow(xPos - ballV[j]->x, 2.0) + pow(yPos - ballV[j]->y, 2.0)) <= 2.f*ballDia)
						collision = true;
				}
				tryCount++;
			}
			
			ball->setPosition(xPos, yPos);
			ballV.push_back(ball);
		}
		
		for (int i = 0; i < ballV.size(); i++) {
			quadTree->addParticle(ballV[i], true);
		}
		
	}
	
	// Create a spherical cloud of particles
	void createCloud(double x, double y, double rad, double vel, double dir,
										double ballDia, double ballDensity, double springRate, double rebEff,
										double attrRate, double attrRad, bool stationary, bool ovrWrite) {
		std::vector<int> list;
		double ballRad = ballDia/2.f;
		int tempListPos;
		
		double xIt, yIt;
		int rows = (rad)/(ballDia*sin(PI60));
		yIt = y - rows*ballDia*sin(PI60);
		int j = 0;
		while (abs(yIt - y) < rad) {
		
			if (j%2) xIt = x + ballDia*cos(PI60);
			else xIt = x;
			while (sqrt(pow(xIt - x, 2.0) + pow(yIt - y, 2.0)) < rad) {
				tempListPos = createParticle(xIt, yIt, 0, 0, ballDia, ballDensity, springRate, rebEff, attrRate, attrRad, true, ovrWrite);
				if (tempListPos >= 0) list.push_back(tempListPos);
				xIt += ballDia;
			}
			if (j%2) xIt = x - ballDia*cos(PI60);
			else xIt = x - ballDia;
			while (sqrt(pow(xIt - x, 2.0) + pow(yIt - y, 2.0)) < rad) {
				tempListPos = createParticle(xIt, yIt, 0, 0, ballDia, ballDensity, springRate, rebEff, attrRate, attrRad, true, ovrWrite);
				if (tempListPos >= 0) list.push_back(tempListPos);
				xIt -= ballDia;
			}
			yIt += ballDia*sin(PI60);
			j++;
		}
		double velX = vel*cos(dir);
		double velY = vel*sin(dir);
		for (int j = 0; j < list.size(); j++) {
			if (!stationary) ballV[list[j]]->stationary = false;
			ballV[list[j]]->xVel = velX;
			ballV[list[j]]->yVel = velY;
		}
	}

	int createParticle(double xPos, double yPos, double vel, double dir,
											double ballDia, double ballDensity, double springRate, double rebEff,
											double attrRate, double attrRad, bool stationary, bool ovrWrite) {

		double radius = ballDia/2.f;
		bool collision = false;
		if (xPos > *resX - radius || xPos < radius || yPos > *resY - radius || yPos < radius) {
			collision = true;
		}
		else if (!ovrWrite) {
			for (int j = 0; j < ballV.size(); j++) {
				if (ballV[j]->alive) {
					if (sqrt(pow(xPos - ballV[j]->x, 2.0) + pow(yPos - ballV[j]->y, 2.0)) + 0.001 < radius + ballV[j]->radius) {
						collision = true;
					}
				}
			}
			for (int k = 0; k < bhV.size(); k++) {
				if (bhV[k].active) {
					if (sqrt(pow(xPos - bhV[k].x, 2.0) + pow(yPos - bhV[k].y, 2.0)) + 0.001 < radius + bhV[k].radius) {
						collision = true;
					}
				}
			}
		}
		if (!collision && ballV.size() < MAX_PARTICLES) {
			int i = 1;
			bool inactiveBall = false;
			while(i < ballV.size()) {
				if(!ballV[i]->alive) {
					inactiveBall = true;
					break;
				}
				i++;
			}
			
			if (inactiveBall) {
				ballV[i]->setSize(ballDia);
				ballV[i]->setMass(ballDensity);
				ballV[i]->setColor(rand()%255, rand()%255, rand()%255);
				ballV[i]->springRate = springRate;
				ballV[i]->reboundEfficiency = rebEff;
				ballV[i]->setSticky(attrRad, attrRate);
				ballV[i]->setPosition(xPos, yPos);
				ballV[i]->alive = true;
				ballV[i]->stationary = stationary;
				return i;
			}
			else {
				z::Ball *ball;
				ball = new Ball();
				ball->setSize(ballDia);
				ball->setMass(ballDensity);
				ball->setColor(std::rand()%255, std::rand()%255, std::rand()%255);
				ball->springRate = springRate;
				ball->reboundEfficiency = rebEff;
				ball->setSticky(attrRad, attrRate);
				ball->setPosition(xPos, yPos);
				ball->stationary = stationary;
				quadTree->addParticle(ball, true);
				ballV.push_back(ball);
				return ballV.size() - 1;
			}
		}
		else return -1;
	}
	
	void createBH(int x, int y, double surfaceAccel, int diameter, InteractionSetting interact) {
		if (bhV.size() < MAX_BH) {
			int i = 1;
			bool inactiveBH = false;
			while(i < bhV.size()) {
				if(!bhV[i].active) {
					inactiveBH = true;
					break;
				}
				i++;
			}
			
			if (inactiveBH) {
				bhV[i].setSize(diameter);
				bhV[i].setAttraction(surfaceAccel);
				bhV[i].setPosition(x, y);
				bhV[i].interact = interact;
				bhV[i].active = true;
			}
			else {
				z::BlackHole bhTemp = BlackHole(x, y, surfaceAccel, diameter, interact);
				bhTemp.active = true;
				bhV.push_back(bhTemp);
			}
		}
	}
	
	///////////////////////////
	// Particle Manipulation //
	///////////////////////////
	
	int size() {
		return ballV.size();
	}
	
	void cleanParticles() {
		int front = 0;
		int back = ballV.size() - 1;
		while (front < back) {
			while (ballV[front]->alive && front < ballV.size()) front++; // Find dead ball
			while (!ballV[back]->alive && back >= 0) back--; // Find live ball
			if (front < back) std::swap(ballV[front],ballV[back]); // Swap
		}
		
		back = ballV.size();
		while (back >= 0) {
			if (!ballV[back-1]->alive) back--;
			else break;
		}
		if (back < ballV.size()) {
			int eraseStart = (back <= 50)?50:back;
			for (int k = eraseStart; k < ballV.size(); k++) delete ballV[k];
			ballV.erase(ballV.begin()+eraseStart, ballV.end());
		}
	}
	
	void cleanBH() {
		int front = 1;
		int back = bhV.size() - 1;
		while (front < back) {
			while (bhV[front].active && front < bhV.size()) front++; // Find dead ball
			while (!bhV[back].active && back >= 1) back--; // Find live ball
			if (front < back) std::swap(bhV[front],bhV[back]); // Swap
		}
		
		back = bhV.size();
		while (back >= 1) {
			if (!bhV[back-1].active) back--;
			else break;
		}
		if (back < bhV.size()) {
			bhV.erase(bhV.begin()+back, bhV.end());
		}
	}
	
	void zeroVel() {
		for (int i = 0; i < ballV.size(); i++) {
			if (ballV[i]->alive) {
				ballV[i]->xVel = 0;
				ballV[i]->yVel = 0;
			}
		}
	}
	
	void clearParticles() {
		for (int i = 0; i < ballV.size(); i++) {
			ballV[i]->alive = false;
		}
		for (int j = 1; j < bhV.size(); j++) {
			bhV[j].active = false;
		}
	}
	
	void immobilizeCloud(double x, double y, double rad) {
		prevX = x;
		prevY = y;
		for (int i = 0; i < ballV.size(); i++) {
			if (ballV[i]->alive) {
				double dist = sqrt(pow(ballV[i]->x - x, 2.0) + pow(ballV[i]->y - y, 2.0));
				if (dist <= rad) {
					listParticles.push_back(i);
					ballV[i]->stationary = true;
				}
			}
		}
		for (int j = 0; j < bhV.size(); j++) {
			if (bhV[j].active) {
				double dist = sqrt(pow(bhV[j].x - x, 2.0) + pow(bhV[j].y - y, 2.0));
				if (dist <= rad) {
					listBH.push_back(j);
				}
			}
		}
	}
	
	// Called after immobilizeCloud
	void moveCloud(double x, double y) {
		double deltaX = x - prevX;
		double deltaY = y - prevY;
		if (deltaX != 0 || deltaY != 0) {
			for (int j = 0; j < listParticles.size(); j++) {
				ballV[listParticles[j]]->xMove += deltaX;
				ballV[listParticles[j]]->yMove += deltaY;
			}
			for (int k = 0; k < listBH.size(); k++) {
				bhV[listBH[k]].xMove += deltaX;
				bhV[listBH[k]].yMove += deltaY;
			}
		}
		prevX = x;
		prevY = y;
	}
	
	// Called after immobilizeCloud
	void mobilizeCloud() {
		for (int j = 0; j < listParticles.size(); j++) {
			ballV[listParticles[j]]->stationary = false;
		}
		listParticles.clear();
		listBH.clear();
	}
	
	// Erase a spherical region of particles
	void deactivateCloud(double x, double y, double rad) {
		for (int i = 0; i < ballV.size(); i++) {
			if (ballV[i]->alive) {
				double dist = sqrt(pow(ballV[i]->x - x, 2.0) + pow(ballV[i]->y - y, 2.0));
				if (dist <= rad) {
					ballV[i]->alive = false;
				}
			}
		}
		for (int j = 1; j < bhV.size(); j++) {
			if (bhV[j].active) {
				double dist = sqrt(pow(bhV[j].x - x, 2.0) + pow(bhV[j].y - y, 2.0));
				if (dist <= rad) {
					bhV[j].active = false;
				}
			}
		}
	}
	
	/////////////
	// Physics //
	/////////////
	
	void quadSortParticles(unsigned int iStart, unsigned int iStop) {
		for (int i = iStart; i < iStop; i++) {
			ballV[i]->quadResidence->sortParticle(ballV[i]->getID());
		}
	}
	
	void addPhysics(unsigned int iStart, unsigned int iStop) {
		//iStart = (iStart < 0)?(0):((iStart > ballV.size())?ballV.size():iStart);
		//iStop = (iStop > ballV.size())?ballV.size():iStop;
				
		int bhVsize = bhV.size();
		
		// Singular physics
		for ( int i = iStart; i < iStop; i++ ) {
			if (ballV[i]->stationary == false) {
				// Particle-boundary collisions
				if (boundWalls) {
					if (ballV[i]->x > *resX - ballV[i]->radius) {
						// Ball linear spring rate w/ wall rebound efficiency
						ballV[i]->xVel += ((*resX - ballV[i]->radius) - ballV[i]->x)*ballV[i]->springRate*
						((ballV[i]->xVel < 0) ? ballV[i]->reboundEfficiency : 1.0)*(*tickTime);
					}
					else if (ballV[i]->x < ballV[i]->radius) {    
						ballV[i]->xVel += (ballV[i]->radius - ballV[i]->x)*ballV[i]->springRate*
						((ballV[i]->xVel > 0) ? ballV[i]->reboundEfficiency : 1.0)*(*tickTime);
					}
				}
				if (ballV[i]->y > *resY - ballV[i]->radius) {
					if (boundFloor) {
						ballV[i]->yVel += ((*resY - ballV[i]->radius) - ballV[i]->y)*ballV[i]->springRate*
						((ballV[i]->yVel < 0) ? ballV[i]->reboundEfficiency : 1.0)*(*tickTime);
					}
				}
				else if (ballV[i]->y < ballV[i]->radius) {
					if (boundCeiling) {
						ballV[i]->yVel += (ballV[i]->radius - ballV[i]->y)*ballV[i]->springRate*
						((ballV[i]->yVel > 0) ? ballV[i]->reboundEfficiency : 1.0)*(*tickTime);
					}
				}
				else {
					// Linear gravity
					ballV[i]->yVel += (linGravity)*(*tickTime);
				}
				
				// Black hole effects
				for (int k = 0; k < bhVsize; k++) {
					if (bhV[k].active == true) {
						double term;
						double dist = sqrt(pow(ballV[i]->x - bhV[k].x, 2.f) + pow(ballV[i]->y - bhV[k].y, 2.f));
						if (bhV[k].interact == COLLISION && dist < ballV[i]->radius + bhV[k].radius) { 
							term = ballV[i]->springRate*(ballV[i]->radius + bhV[k].radius - dist)*(*tickTime);
							
							ballV[i]->xVel += ((ballV[i]->x - bhV[k].x)/dist)*term;
							ballV[i]->yVel += ((ballV[i]->y - bhV[k].y)/dist)*term;
						}
						else {
							if (dist > bhV[k].radius) {
								term = (bhV[k].centerAccel/pow(dist, 2))*(*tickTime);
							}
							else {
								if (bhV[k].interact = DESTRUCTION ){
									ballV[i]->alive = false;
								}
								else {
									term = bhV[k].surfaceAccel*(*tickTime);
								}
							}

							ballV[i]->xVel += ((bhV[k].x - ballV[i]->x)/dist)*term;
							ballV[i]->yVel += ((bhV[k].y - ballV[i]->y)/dist)*term;
						}
					}
				}
			}
		
			ballV[i]->update();
		}
		if (iStart == 0) {
			for (int k = 0; k < bhVsize; k++) {
				bhV[k].update();
			}
		}
	}

	void collisonUpdate(unsigned int iStart, unsigned int iStop) {
		//iStart = (iStart < 0)?(0):((iStart > ballV.size())?ballV.size():iStart);
		//iStop = (iStop > ballV.size())?ballV.size():iStop;
				
		int bVsize = ballV.size();
		int bhVsize = bhV.size();
		
		for (int i = iStart; i < iStop; i++) {
			if (ballV[i]->alive) {
				
				// Inter-particle collisions
				if (particleCollisions) {
					for (int j = i+1; j < bVsize; j++) {
						if (ballV[j]->alive) {
						
							// Distance between the two points
							double dist = sqrt(pow(ballV[i]->x - ballV[j]->x, 2.0) + pow(ballV[i]->y - ballV[j]->y, 2.0));
							if (dist == 0) dist = 0.01; // Remove divide by zero errors
							
							double centerDist = ballV[i]->radius + ballV[j]->radius;
							
							
							// Check for particle collision
							if (dist < centerDist) { 
								double force = ((ballV[i]->springRate + ballV[i]->springRate)/2.f)
															*(centerDist - dist)*(*tickTime);
								double forceVect;
								
								// If ball centers collide, then average their momentum
								//(((ballV[i]->x < ballV[j]->x && ballV[i]->xVel < ballV[j]->xVel) ||
								//		(ballV[i]->x > ballV[j]->x && ballV[i]->xVel > ballV[j]->xVel))
								if (dist < centerDist*0.2) {
									if((ballV[i]->x < ballV[j]->x && ballV[i]->xVel > ballV[j]->xVel) ||
										(ballV[i]->x > ballV[j]->x && ballV[i]->xVel < ballV[j]->xVel)) {
										
										double momentum = (ballV[i]->xVel*ballV[i]->mass + ballV[j]->xVel*ballV[j]->mass)/2.0;
										
										ballV[i]->xVel = momentum/ballV[i]->mass;
										ballV[j]->xVel = momentum/ballV[j]->mass;
									}
									if((ballV[i]->y < ballV[j]->y && ballV[i]->yVel > ballV[j]->yVel) ||
										(ballV[i]->y > ballV[j]->y && ballV[i]->yVel < ballV[j]->yVel)) {
										
										double momentum = (ballV[i]->yVel*ballV[i]->mass + ballV[j]->yVel*ballV[j]->mass)/2.0;
										
										ballV[i]->yVel = momentum/ballV[i]->mass;
										ballV[j]->yVel = momentum/ballV[j]->mass;
									}									
								}
								forceVect = ((ballV[i]->x - ballV[j]->x)/dist)*force*
									(((ballV[i]->x < ballV[j]->x && ballV[i]->xVel < ballV[j]->xVel) ||
									(ballV[i]->x > ballV[j]->x && ballV[i]->xVel > ballV[j]->xVel)) ? ballV[i]->reboundEfficiency : 1.0);
								ballV[i]->xVel += forceVect/ballV[i]->mass;
								ballV[j]->xVel -= forceVect/ballV[j]->mass;
								forceVect = ((ballV[i]->y - ballV[j]->y)/dist)*force*
									(((ballV[i]->y < ballV[j]->y && ballV[i]->yVel < ballV[j]->yVel) ||
									(ballV[i]->y > ballV[j]->y && ballV[i]->yVel > ballV[j]->yVel)) ? ballV[i]->reboundEfficiency : 1.0);
								ballV[i]->yVel += forceVect/ballV[i]->mass;
								ballV[j]->yVel -= forceVect/ballV[j]->mass;
							}
							else if (particleStickyness && dist < centerDist + std::max(ballV[i]->attrRad, ballV[j]->attrRad)) {
								
								// Figure out which attraction rates to use
								double attractRate = 0;
								if (dist < centerDist + ballV[i]->attrRad) {
									attractRate += ballV[i]->attrRate;
								}
								if (dist < centerDist + ballV[i]->attrRad) {
									attractRate += ballV[j]->attrRate;
								}
								
								double force = attractRate*(*tickTime)/std::pow(dist, 2.f);
								double forceVect;
								
								forceVect = ((ballV[i]->x - ballV[j]->x)/dist)*force;
								ballV[i]->xVel -= forceVect/ballV[i]->mass;
								ballV[j]->xVel += forceVect/ballV[j]->mass;
								forceVect = ((ballV[i]->y - ballV[j]->y)/dist)*force;
								ballV[i]->yVel -= forceVect/ballV[i]->mass;
								ballV[j]->yVel += forceVect/ballV[j]->mass;
								
								force = (ballV[i]->x - ballV[j]->x);
							
							}
						}
					}
				}
			}
		}
	}
		
	void draw(sf::RenderWindow* mainWindow) {
		int tempCount = 0;
		double maxVel = 0;
		double vel;
		
		int bVsize = ballV.size();
		int bhVsize = bhV.size();
		
		// Draw all particles in ball vector
		for (int i = 0; i < bVsize; i++ ) {
			if (ballV[i]->alive) {
				tempCount++;
				vel = sqrt(pow(ballV[i]->xVel, 2.0) + pow(ballV[i]->yVel, 2.0));
				if (vel > maxVel) maxVel = vel;
				mainWindow->draw(ballV[i]->ballShape);
				;
			}
		}
		ballAlive = tempCount;
		if (bVsize - ballAlive >= PARTICLE_CLEAN && bVsize - PARTICLE_CLEAN > 50) cleanParticles();
		
		maxParticleVel = maxVel;
		
		tempCount = 0;
		for (int i = 0; i < bhVsize; i++ ) {
			if (bhV[i].active) {
				//std::cout << "yes\n";
				tempCount++;
				mainWindow->draw(bhV[i].ballShape);
			}
		}
		bhAlive = tempCount;
		if (bhVsize - bhAlive >= BH_CLEAN && bhVsize - BH_CLEAN > 1) cleanBH();

	}
};

}