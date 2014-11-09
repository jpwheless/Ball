#ifndef BALL_HPP
#define BALL_HPP

#define DRAG_FILT 10.0

namespace z {

class Quad;

class Ball {
private:
	unsigned int id; // This never changes and is unique after using the setID function

public:
	double x, y;
	double xMove, yMove;
	double xVel, yVel;
	double diameter, radius;
	double mass;
	double springRate, reboundEfficiency;
	double attrRad, attrRate;
	double bound;
	sf::CircleShape ballShape;
	bool alive, stationary;
	
	static bool *boundCeiling;
	static bool *boundWalls;
	static bool *boundFloor;
	static bool *sticky;
	static double *tickTime;
	static int *resX;
	static int *resY;
	
	Quad *quadResidence;
	
	//std::atomic_bool mStopEvent{false};

	Ball() {		
		alive = true;
		stationary = false;
		x = y = xVel = yVel = 0;
		setSize(10.0);
		setColor(255, 255, 255);
		mass = 1.0;
		
		quadResidence = NULL;
		
		id = 0;
		
		setID();
	}

	void update() {
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

	void setPosition( double xIn, double yIn ){
		x = xIn;
		y = yIn;
		xMove = xIn;
		yMove = yIn;
		ballShape.setPosition(x - radius, y - radius);
	}
	
	void setSize(int diameter){
		this->diameter = diameter;
		radius = diameter / 2.0;
		ballShape.setRadius(radius);
	}
	
	void setMass(double density) {
		double area = 3.14159265359*pow(radius, 2.0);
		mass = area*density;
	}
	
	void setSticky(double attrRadT, double attrRateT) {
		attrRate = attrRateT*pow(radius, 2.0); // Convert to center attr rate for physics
		attrRad = attrRadT;
	}
	
	void setColor(int r, int g, int b) {
		ballShape.setFillColor(sf::Color(r, g, b));
	}
	
	void setID() {
		static unsigned long int idCounter = 1;
		
		if (id == 0) {
			id = idCounter;
			idCounter++;
		}
	}
	
	unsigned long int getID() {
		return id;
	}
	
	// Fills array with xMin, xMax, yMin, yMax of bounding box
	void getBounds(double (&boundArray)[4]) {
		double dist = radius + (*sticky) ? attrRad : 0.0;
		boundArray[0] = x - dist;
		boundArray[1] = x + dist;
		boundArray[2] = y - dist;
		boundArray[3] = y + dist;
	}
};

bool *Ball::boundCeiling;
bool *Ball::boundWalls;
bool *Ball::boundFloor;
bool *Ball::sticky;
double *Ball::tickTime;
int *Ball::resX;
int *Ball::resY;

}


#endif