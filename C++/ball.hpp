#ifndef BALL_HPP
#define BALL_HPP

#define DRAG_FILT 10.0


#define STICKY_NONE 10
#define STICKY_NONE 10
#define STICKY_NONE 10

#define BASE_SPR_RATE 50000.0
#define DEFAULT_REB_EFF 0.9
#define BASE_ATTR_RATE 50000.0
#define DEFAULT_ATTR_RAD 5.0

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#include <SFML/Graphics.hpp>
#include "quad.hpp"

namespace z {

//class Quad;

class Ball {
public:
	unsigned int id; // This never changes and is unique after using the setID function
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
	
	int diameterClass, densityClass;
	
	static bool *boundCeiling;
	static bool *boundWalls;
	static bool *boundFloor;
	static bool *sticky;
	static double *tickTime;
	static int *resX;
	static int *resY;
	
	static const double densityTable[];
	static const double diameterTable[];
	
	Quad *quadResidence;
	
	Ball(int, int);	
	void update();
	void setPosition(double, double);
	void setSize(int);
	void setMass(int);
	void setColor(int, int, int);
	void setID();
	void updateBounds();
};
}

#endif