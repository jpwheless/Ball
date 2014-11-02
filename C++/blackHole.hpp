#ifndef BLACK_HOLE_HPP
#define BLACK_HOLE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

#define MOUSE_FILT 30.0

namespace z {

enum InteractionSetting {
	NO_COLLISION,
	DESTRUCTION,
	COLLISION
};

class BlackHole {
public:
	double x, y;
	double xMove, yMove;
	double surfaceAccel, centerAccel;
	double diameter, radius;
	bool active;
	InteractionSetting interact;
	
	sf::CircleShape ballShape;
	
	static double *tickTime;

	BlackHole(double x, double y, double surfaceAccel, int diameter, InteractionSetting interact) {
		setAttraction(surfaceAccel);
		setSize(diameter);
		setPosition(x, y);
		active = false;
		this->interact = interact;
	}

	void filteredMove(int mouseX, int mouseY) {
		xMove = mouseX;
		yMove = mouseY;
	}
	
	void update() {
		if (active) {
			x += MOUSE_FILT*(*tickTime)*(xMove - x);
			y += MOUSE_FILT*(*tickTime)*(yMove - y);
			ballShape.setPosition(x - radius, y - radius);
		}
	}
	
	void setPosition(int x, int y) {
		this->x = x;
		this->y = y;
		this->xMove = x;
		this->yMove = y;
		ballShape.setPosition(x - radius, y - radius);
	}

	void setSize(int diameter){
		this->diameter = diameter;
		radius = diameter/2.0;
		ballShape.setRadius(radius);
		centerAccel = surfaceAccel*pow(radius, 2);
	}
	
	void setAttraction(double surfaceAccel) {
		this->surfaceAccel = surfaceAccel;
		centerAccel = surfaceAccel*pow(radius, 2);
		if (surfaceAccel < 0) setColor(255, 255, 255);
		else if (surfaceAccel == 0) setColor(127, 127, 127);
		else setColor(0, 0, 0);
	}
	
	void setColor(int r, int g, int b) {
		ballShape.setOutlineColor(sf::Color::White);
		ballShape.setOutlineThickness(1);
		ballShape.setFillColor(sf::Color(r, g, b)); 
	}
};

double *BlackHole::tickTime;

}

#endif