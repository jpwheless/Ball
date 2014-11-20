#ifndef BALL_HPP
#define BALL_HPP

#define DRAG_FILT 10.0


#define BASE_SPR_RATE 50000.0
#define DEFAULT_REB_EFF 0.9
#define BASE_ATTR_RATE 25000.0
#define DEFAULT_ATTR_RAD 5.0

#include <SFML/Graphics.hpp>
#include "quad.hpp"

namespace z {

//class Quad;

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
	double xMin, xMax, yMin, yMax;
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
	
	Ball(double, double);	
	void update();
	void setPosition(double, double);
	void setSize(double);
	void setMass(double);
	void setColor(int, int, int);
	void setID();
	unsigned long int getID();
	void updateBounds();
};
}

#endif