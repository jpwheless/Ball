#ifndef BALL_HPP
#define BALL_HPP

#include <atomic>

namespace z {

class Ball {
public:
	float x, y;
	float xVel, yVel;
	float diameter, radius;
	float springRate, reboundEfficiency;
	float attrRad, attrRate;
	sf::CircleShape ballShape;
	bool alive;
	
	//std::atomic<float> temp = 0;

	Ball() {
		alive = true;
		x = y = xVel = yVel = 0;
		setSize(10.0);
		setColor(255, 255, 255);
		
		//++temp;
	}

	void update( float frameTime, const int& xRes, const int& yRes, const bool& bound){
		if( alive ) {
			x += xVel * frameTime;
			y += yVel * frameTime;
			ballShape.setPosition( x - radius, y - radius );
			
			// Boundary conditions
			if (x < -radius || x > xRes + radius) {
				if (bound == true) {
					x = (x < radius)?(radius):((x > xRes-radius)?(xRes-radius):x);
				}
				else alive = false;
			}
			if (y < -radius || y > yRes + radius) {
				if (bound == true) {
					y = (y < radius)?(radius):((y > yRes-radius)?(yRes-radius):y);
				}
				else alive = false;
			}
		}
	}

	void setPosition( float xIn, float yIn ){
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