#ifndef BALL_HPP
#define BALL_HPP

#define DRAG_FILT 10.0

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
	
	Ball();	
	void update();
	void setPosition(double, double);
	void setSize(int);
	void setMass(double);
	void setSticky(double, double);
	void setColor(int, int, int);
	void setID();
	unsigned long int getID();
	void getBounds(double boundArray[4]);
};
}

#endif