#include "particles.hpp"

namespace z {

	/////////////////
	// Constructor //
	/////////////////
	
	Particles::Particles(int *resXT, int *resYT, double *tickTimeT, double linGravityT) {
		resX = resXT;
		resY = resYT;
		tickTime = tickTimeT;
		linGravity = linGravityT;
		
		Quad::particles = this;
		quadTree = new Quad(NULL, 0, LEVELS, 0, 0, *resX, 0, *resY);
		
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
		
	///////////////////////
	// Particle Creation //
	///////////////////////

	// Populate the window with balls in random locations
	void Particles::createInitBalls(unsigned int numBalls, int ballDia, int ballDensity) {
		// Create number of starting balls at random locations
		for (unsigned int i = 0; i < numBalls; i++ ) {
			z::Ball *ball;
			ball = new Ball(ballDia, ballDensity);
			
			double xPos, yPos;
			int tryCount = 0;
			bool collision = true;

			// Make sure ball doesn't collide with another upon start
			while (collision && tryCount <= NUM_TRIES) {
				xPos = randDouble(ball->radius, *resX - ball->radius);
				yPos = randDouble(ball->radius, *resY - ball->radius);
				collision = false;
				for (unsigned int j = 0; j < i; j++) {
					if (sqrt(pow(xPos - ballV[j]->x, 2.0) + pow(yPos - ballV[j]->y, 2.0)) <= 2.f*ballDia)
						collision = true;
				}
				tryCount++;
			}
			
			ball->setPosition(xPos, yPos);
			ballV.push_back(ball);
		}
		
		for (unsigned int i = 0; i < ballV.size(); i++) {
			quadTree->addParticle(ballV[i], true);
		}
	}

	// Create a spherical cloud of particles
	void Particles::createCloud(double x, double y, double rad, double vel, double dir,
										int ballDia, int ballDensity, bool stationary, bool force) {
		std::vector<int> list;
		int tempListPos;
		
		double xIt, yIt;
		int rows = (rad)/(ballDia*sin(PI60));
		yIt = y - rows*ballDia*sin(PI60);
		int j = 0;
		while (abs(yIt - y) < rad) {
		
			if (j%2) xIt = x + ballDia*cos(PI60);
			else xIt = x;
			while (sqrt(pow(xIt - x, 2.0) + pow(yIt - y, 2.0)) < rad) {
				tempListPos = createParticle(xIt, yIt, 0, 0, ballDia, ballDensity, true, force);
				if (tempListPos >= 0) list.push_back(tempListPos);
				xIt += ballDia;
			}
			if (j%2) xIt = x - ballDia*cos(PI60);
			else xIt = x - ballDia;
			while (sqrt(pow(xIt - x, 2.0) + pow(yIt - y, 2.0)) < rad) {
				tempListPos = createParticle(xIt, yIt, 0, 0, ballDia, ballDensity, true, force);
				if (tempListPos >= 0) list.push_back(tempListPos);
				xIt -= ballDia;
			}
			yIt += ballDia*sin(PI60);
			j++;
		}
		double velX = vel*cos(dir);
		double velY = vel*sin(dir);
		for (unsigned int j = 0; j < list.size(); j++) {
			if (!stationary) ballV[list[j]]->stationary = false;
			ballV[list[j]]->xVel = velX;
			ballV[list[j]]->yVel = velY;
		}
	}
	
	// Singular particle creation with collision checking
	int Particles::createParticle(double xPos, double yPos, double vel, double dir,
											int ballDia, int ballDensity, bool stationary, bool force) {

		double radius = Ball::diameterTable[((ballDia)<(0)?(0):((ballDia)>(2)?(2):(ballDia)))]/2.0;
		bool collision = false;
		if (xPos > *resX - radius || xPos < radius || yPos > *resY - radius || yPos < radius) {
			collision = true;
		}
		else if (!force) {
			for (unsigned int j = 0; j < ballV.size(); j++) {
				if (ballV[j]->alive) {
					if (sqrt(pow(xPos - ballV[j]->x, 2.0) + pow(yPos - ballV[j]->y, 2.0)) + 0.001 < radius + ballV[j]->radius) {
						collision = true;
					}
				}
			}
			for (unsigned int k = 0; k < bhV.size(); k++) {
				if (bhV[k].active) {
					if (sqrt(pow(xPos - bhV[k].x, 2.0) + pow(yPos - bhV[k].y, 2.0)) + 0.001 < radius + bhV[k].radius) {
						collision = true;
					}
				}
			}
		}
		if (!collision && ballV.size() < MAX_PARTICLES) {
			unsigned int i = 1;
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
				ballV[i]->setPosition(xPos, yPos);
				ballV[i]->alive = true;
				ballV[i]->stationary = stationary;
				return i;
			}
			else {
				z::Ball *ball;
				ball = new Ball(ballDia, ballDensity);
				ball->setPosition(xPos, yPos);
				ball->stationary = stationary;
				quadTree->addParticle(ball, true);
				ballV.push_back(ball);
				return ballV.size() - 1;
			}
		}
		else return -1;
	}
	
		void Particles::createBH(int x, int y, double surfaceAccel, int diameter, InteractionSetting interact) {
		if (bhV.size() < MAX_BH) {
			unsigned int i = 1;
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
	
	// Erase dead particles
	void Particles::cleanParticles() {
		int frontSwap = 0;
		int backSwap = ballV.size() - 1;
		while (frontSwap < backSwap) {
			while (frontSwap < ballV.size() && ballV[frontSwap]->alive) frontSwap++; // Find dead ball
			while (backSwap > 0 && !ballV[backSwap]->alive) backSwap--; // Find live ball
			if (frontSwap < backSwap) std::swap(ballV[frontSwap],ballV[backSwap]); // Swap
		}
		
		backSwap = ballV.size();
		while (backSwap > 0) {
			if (!ballV[backSwap-1]->alive) backSwap--;
			else break;
		}
		if (backSwap < ballV.size()) {
			int eraseStart = (backSwap < 50)?50:backSwap;
			for (unsigned int k = eraseStart; k < ballV.size(); k++)
				ballV[k]->quadResidence->checkIfResident(ballV[k]->getID(), true);
			ballV.erase(ballV.begin()+eraseStart, ballV.end());
		}
	}
	
	void Particles::cleanBH() {
		int frontSwap = 1;
		int backSwap = bhV.size() - 1;
		while (frontSwap < backSwap) {
			while (frontSwap < bhV.size() && bhV[frontSwap].active) frontSwap++; // Find dead ball
			while (backSwap > 0 && !bhV[backSwap].active) backSwap--; // Find live ball
			if (frontSwap < backSwap) std::swap(bhV[frontSwap],bhV[backSwap]); // Swap
		}
		
		backSwap = bhV.size();
		while (backSwap > 0) {
			if (!bhV[backSwap-1].active) backSwap--;
			else break;
		}
		if (backSwap < bhV.size()) {
			bhV.erase(bhV.begin()+backSwap, bhV.end());
		}
	}
	
	void Particles::cleanQuad() {
		quadTree->cleanResidentList();
	}
	
	void Particles::zeroVel() {
		for (unsigned int i = 0; i < ballV.size(); i++) {
			if (ballV[i]->alive) {
				ballV[i]->xVel = 0;
				ballV[i]->yVel = 0;
			}
		}
	}
	
	void Particles::clearParticles() {
		for (unsigned int i = 0; i < ballV.size(); i++) {
			ballV[i]->alive = false;
		}
		for (unsigned int j = 1; j < bhV.size(); j++) {
			bhV[j].active = false;
		}
	}

	void Particles::immobilizeCloud(double x, double y, double rad) {
		prevX = x;
		prevY = y;
		for (unsigned int i = 0; i < ballV.size(); i++) {
			if (ballV[i]->alive) {
				double dist = sqrt(pow(ballV[i]->x - x, 2.0) + pow(ballV[i]->y - y, 2.0));
				if (dist <= rad) {
					listParticles.push_back(i);
					ballV[i]->stationary = true;
				}
			}
		}
		for (unsigned int j = 0; j < bhV.size(); j++) {
			if (bhV[j].active) {
				double dist = sqrt(pow(bhV[j].x - x, 2.0) + pow(bhV[j].y - y, 2.0));
				if (dist <= rad) {
					listBH.push_back(j);
				}
			}
		}
	}
	
	// Called after immobilizeCloud
	void Particles::moveCloud(double x, double y) {
		double deltaX = x - prevX;
		double deltaY = y - prevY;
		if (deltaX != 0 || deltaY != 0) {
			for (unsigned int j = 0; j < listParticles.size(); j++) {
				ballV[listParticles[j]]->xMove += deltaX;
				ballV[listParticles[j]]->yMove += deltaY;
			}
			for (unsigned int k = 0; k < listBH.size(); k++) {
				bhV[listBH[k]].xMove += deltaX;
				bhV[listBH[k]].yMove += deltaY;
			}
		}
		prevX = x;
		prevY = y;
	}
	
	// Called after immobilizeCloud
	void Particles::mobilizeCloud() {
		for (unsigned int j = 0; j < listParticles.size(); j++) {
			ballV[listParticles[j]]->stationary = false;
		}
		listParticles.clear();
		listBH.clear();
	}
	
	// Erase a spherical region of particles
	void Particles::deactivateCloud(double x, double y, double rad) {
		for (unsigned int i = 0; i < ballV.size(); i++) {
			if (ballV[i]->alive) {
				double dist = sqrt(pow(ballV[i]->x - x, 2.0) + pow(ballV[i]->y - y, 2.0));
				if (dist <= rad) {
					ballV[i]->alive = false;
				}
			}
		}
		for (unsigned int j = 1; j < bhV.size(); j++) {
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
	
	// Sort particles within quad tree
	void Particles::quadSortParticles(unsigned int iStart, unsigned int iStop) {
		for (unsigned int i = iStart; i < iStop; i++) {
			ballV[i]->quadResidence->sortParticle(ballV[i]);
		}
	}
	
	// Do optimized collision searching
	void Particles::quadCollideParticles(unsigned int iStart, unsigned int iStop) {
		if (particleCollisions) {
			for (unsigned int i = iStart; i < iStop; i++) {
				if (ballV[i]->alive) {
					ballV[i]->quadResidence->collideParticles(ballV[i], true);
				}
			}
		}
	}

	void Particles::addPhysics(unsigned int iStart, unsigned int iStop) {
		unsigned int bhVsize = bhV.size();
		
		// Singular physics
		for (unsigned int i = iStart; i < iStop; i++ ) {
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
				for (unsigned int k = 0; k < bhVsize; k++) {
					if (bhV[k].active == true) {
						double term = 0;
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
								if (bhV[k].interact == DESTRUCTION){
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
			for (unsigned int k = 0; k < bhVsize; k++) {
				bhV[k].update();
			}
		}
	}
	
	// Assumes that particleCollisions and both balls are alive
	void Particles::collisonUpdate(Ball *ballA, Ball *ballB) {
		
		// Distance between the two points
		double dist = sqrt(pow(ballA->x - ballB->x, 2.0) + pow(ballA->y - ballB->y, 2.0));
		if (dist == 0) dist = 0.01; // Remove divide by zero errors
		
		double centerDist = ballA->radius + ballB->radius;
		
		
		// Check for particle collision
		if (dist < centerDist) { 
			//double force = ((ballA->springRate + ballB->springRate)/2.f)
			//							*(centerDist - dist)*(*tickTime);
			double force = ((ballA->springRate <= ballB->springRate) ? ballA->springRate : ballB->springRate)
										*(centerDist - dist)*(*tickTime);
			double forceVect;
			
			// If ball centers collide, then average their momentum in an inelastic collision
			//(((ballA->x < ballB->x && ballA->xVel < ballB->xVel) ||
			//		(ballA->x > ballB->x && ballA->xVel > ballB->xVel))
			if (dist < centerDist*0.2) {
				if((ballA->x < ballB->x && ballA->xVel > ballB->xVel) ||
					(ballA->x > ballB->x && ballA->xVel < ballB->xVel)) {
					
					double velocity = (ballA->xVel*ballA->mass + ballB->xVel*ballB->mass)/(ballA->mass + ballB->mass);
					
					ballA->xVel = velocity;
					ballB->xVel = velocity;
				}
				if((ballA->y < ballB->y && ballA->yVel > ballB->yVel) ||
					(ballA->y > ballB->y && ballA->yVel < ballB->yVel)) {
					
					double velocity = (ballA->yVel*ballA->mass + ballB->yVel*ballB->mass)/(ballA->mass + ballB->mass);
					
					ballA->yVel = velocity;
					ballB->yVel = velocity;
				}									
			}
			forceVect = ((ballA->x - ballB->x)/dist)*force*
				(((ballA->x < ballB->x && ballA->xVel < ballB->xVel) ||
				(ballA->x > ballB->x && ballA->xVel > ballB->xVel)) ? ballA->reboundEfficiency : 1.0);
			ballA->xVel += forceVect/ballA->mass;
			ballB->xVel -= forceVect/ballB->mass;
			forceVect = ((ballA->y - ballB->y)/dist)*force*
				(((ballA->y < ballB->y && ballA->yVel < ballB->yVel) ||
				(ballA->y > ballB->y && ballA->yVel > ballB->yVel)) ? ballA->reboundEfficiency : 1.0);
			ballA->yVel += forceVect/ballA->mass;
			ballB->yVel -= forceVect/ballB->mass;
		}
		else if (particleStickyness && dist < centerDist + std::max(ballA->attrRad, ballB->attrRad)) {
			
			// Figure out which attraction rates to use
			double attractRate = 0;
			if (dist < centerDist + ballA->attrRad) {
				attractRate += ballA->attrRate;
			}
			if (dist < centerDist + ballA->attrRad) {
				attractRate += ballB->attrRate;
			}
			
			double force = attractRate*(*tickTime)/std::pow(dist, 2.f);
			double forceVect;
			
			forceVect = ((ballA->x - ballB->x)/dist)*force;
			ballA->xVel -= forceVect/ballA->mass;
			ballB->xVel += forceVect/ballB->mass;
			forceVect = ((ballA->y - ballB->y)/dist)*force;
			ballA->yVel -= forceVect/ballA->mass;
			ballB->yVel += forceVect/ballB->mass;
		}
	}
	
	void Particles::draw(sf::RenderWindow* mainWindow) {
		int tempCount = 0;
		double maxVel = 0;
		double vel;
		
		unsigned int bVsize = ballV.size();
		unsigned int bhVsize = bhV.size();
		
		// Draw all particles in ball vector
		for (unsigned int i = 0; i < bVsize; i++ ) {
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
		for (unsigned int i = 0; i < bhVsize; i++ ) {
			if (bhV[i].active) {
				//std::cout << "yes\n";
				tempCount++;
				mainWindow->draw(bhV[i].ballShape);
			}
		}
		bhAlive = tempCount;
		if (bhVsize - bhAlive >= BH_CLEAN && bhVsize - BH_CLEAN > 1) cleanBH();

	}

}





