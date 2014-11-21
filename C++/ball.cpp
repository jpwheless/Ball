#include "ball.hpp"

namespace z {

	Ball::Ball(int ballDia, int ballDensity) {		
		alive = true;
		stationary = false;
		x = y = xVel = yVel = 0;
		
		densityClass = ballDensity;
		diameterClass = ballDia;
		
		setSize(diameterClass);
		setMass(densityClass);
		
		quadResidence = NULL;
		
		id = 0;
		setID();
	}

	void Ball::update() {
		if (alive) {
			if (stationary) {
				xVel = DRAG_FILT*(xMove - x);
				yVel = DRAG_FILT*(yMove - y);
				x += xVel*(*tickTime);
				y += yVel*(*tickTime);
				ballShape.setPosition(x - radius, y - radius);
			}
			else {
				x += xVel*(*tickTime);
				y += yVel*(*tickTime);
				ballShape.setPosition(x - radius, y - radius);
				
				xMove = x;
				yMove = y;
				
				// Boundary conditions
				/*
				if (*boundCeiling == true) {
					if (x < -50 || x > (*resX) + 50) {
						x = (x < radius)?(radius):((x > (*resX)-radius)?((*resX)-radius):x);
						xVel *= 0.1f;
					}
					if (y < -50 || y > (*resY) + 50) {
						y = (y < radius)?(radius):((y > (*resY)-radius)?((*resY)-radius):y);
						yVel *= 0.1f;
					}
				}
				else {
				*/
				if ((!*boundWalls && (x < -radius || x > (*resX) + radius)) ||
						(!*boundCeiling && y < -radius) || (!*boundFloor && y > (*resY) + radius)) {
					alive = false;
				}
			}
		}
	}

	void Ball::setPosition(double xIn, double yIn){
		x = xIn;
		y = yIn;
		xMove = xIn;
		yMove = yIn;
		ballShape.setPosition(x - radius, y - radius);
	}
	
	void Ball::setSize(int diaClass){
		diaClass = constrain(diaClass, 0, 2);
		diameter = diameterTable[diaClass];
		
		radius = diameter/2.0;
		ballShape.setRadius(radius);
	}
	
	void Ball::setMass(int densClass) {
		densClass = constrain(densClass, 0, 2);
		
		double area = 3.14159265359*pow(radius, 2.0);
		mass = area*(densityTable[densClass]/78.54);
		springRate = BASE_SPR_RATE*densityTable[densClass];
		reboundEfficiency = DEFAULT_REB_EFF;
		
		// Convert to center attr rate for physics
		attrRate = BASE_ATTR_RATE*pow(radius, 2.0)*densityTable[densClass]; 
		attrRad = DEFAULT_ATTR_RAD;
		
		switch (densClass) {
			case 0:
				ballShape.setFillColor(sf::Color(255, 255, 255));
				break;
			case 1:
				ballShape.setFillColor(sf::Color(127, 127, 127));
				break;
			case 2:
				ballShape.setFillColor(sf::Color(0, 0, 0));
				break;
		}
		ballShape.setOutlineThickness(-int(radius*0.4));
		ballShape.setOutlineColor(sf::Color(rand()%255, rand()%255, rand()%255));
	}
		
	void Ball::setColor(int r, int g, int b) {
		ballShape.setFillColor(sf::Color(r, g, b));
	}
	
	void Ball::setID() {
		static unsigned long int idCounter = 1;
		
		if (id == 0) {
			id = idCounter;
			idCounter++;
		}
	}
	
	unsigned long int Ball::getID() {
		return id;
	}
	
	// Fills array with xMin, xMax, yMin, yMax of bounding box
	void Ball::updateBounds() {
		double dist = radius + ((*sticky) ? attrRad : 0.0);
		xMin = x - dist;
		xMax = x + dist;
		yMin = y - dist;
		yMax = y + dist;
	}
	
	bool *Ball::boundCeiling;
	bool *Ball::boundWalls;
	bool *Ball::boundFloor;
	bool *Ball::sticky;
	double *Ball::tickTime;
	int *Ball::resX;
	int *Ball::resY;
	
	const double Ball::densityTable[] = {0.25, 1.0, 4.0};
	const double Ball::diameterTable[] = {10.0, 20.0, 40.0};
	
}
