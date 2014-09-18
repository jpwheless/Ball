#ifndef BALL_HPP
#define BALL_HPP

#include <atomic>

namespace z {

class Ball {
public:
	double x, y;
	double xVel, yVel;
	double diameter, radius;
	double mass;
	double springRate, reboundEfficiency;
	double attrRad, attrRate;
	sf::CircleShape ballShape;
	bool alive, stationary;
	
	//std::atomic<double> temp = 0;

	Ball() {
		alive = true;
		stationary = false;
		x = y = xVel = yVel = 0;
		setSize(10.0);
		setColor(255, 255, 255);
		mass = 1.f;
	}

	void update( const double tickTime, const int xRes, const int yRes, const bool bound){
		if(alive && !stationary) {
			x += xVel * tickTime;
			y += yVel * tickTime;
			ballShape.setPosition(x - radius, y - radius);
			
			// Boundary conditions
			if (x < -50 || x > xRes + 50) {
				if (bound == true) {
					x = (x < radius)?(radius):((x > xRes-radius)?(xRes-radius):x);
					xVel *= 0.1f;
				}
				else alive = false;
			}
			if (y < -50 || y > yRes + 50) {
				if (bound == true) {
					y = (y < radius)?(radius):((y > yRes-radius)?(yRes-radius):y);
					yVel *= 0.1f;
				}
				else alive = false;
			}
		}
		else if (stationary) {
			ballShape.setPosition(x - radius, y - radius);
			xVel = 0;
			yVel = 0;
		}
	}

	void setPosition( double xIn, double yIn ){
		x = xIn;
		y = yIn;
		ballShape.setPosition( x - radius, y - radius );
	}
	
	void setSize( int diameter){
		this->diameter = diameter;
		radius = diameter / 2.f;
		ballShape.setRadius(radius);
	}
	
	void setColor(int r, int g, int b) {
		ballShape.setFillColor(sf::Color(r, g, b)); 
	}
	
	/*
	void setTexture( sf::Texture &texture, int ballDia, int r, int g, int b ){
		diameter = ballDia;
		radius = ballDia / 2.f;
		
		sprite.setOrigin(radius, radius);
		texture.setSmooth(true);
		sprite.setTexture(texture);
		sprite.setScale(diameter/TEXTURE_DIA, diameter/TEXTURE_DIA);
		sprite.setColor(sf::Color(r, g, b));
	}
	*/
};

}

#endif