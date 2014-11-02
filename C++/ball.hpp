#ifndef BALL_HPP
#define BALL_HPP

#define DRAG_FILT 10.0

namespace z {

class Ball {
public:
	double x, y;
	double xMove, yMove;
	double xVel, yVel;
	double diameter, radius;
	double mass;
	double springRate, reboundEfficiency;
	double attrRad, attrRate;
	sf::CircleShape ballShape;
	bool alive, stationary;
		
	static bool *boundCeiling;
	static bool *boundWalls;
	static bool *boundFloor;
	static double *tickTime;
	static int *resX;
	static int *resY;
	
	//std::atomic_bool mStopEvent{false};

	Ball() {		
		alive = true;
		stationary = false;
		x = y = xVel = yVel = 0;
		setSize(10.0);
		setColor(255, 255, 255);
		mass = 1.0;
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
	
	void setColor(int r, int g, int b) {
		ballShape.setFillColor(sf::Color(r, g, b)); 
	}
};

bool *Ball::boundCeiling;
bool *Ball::boundWalls;
bool *Ball::boundFloor;
double *Ball::tickTime;
int *Ball::resX;
int *Ball::resY;

}


#endif