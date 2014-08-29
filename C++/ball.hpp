#ifndef BALL_HPP
#define BALL_HPP

#include <SFML/Graphics.hpp>
#include "vector.hpp"

namespace gp {

class Ball {
public:
	float x, y;
	float xVel, yVel;
	float diameter;
	float radius;
	float springRate;
	sf::Sprite sprite;
	bool alive;

	Ball() {
		alive = true;
		x = y = xVel = yVel = 0;
	}

	void update( float frameTime ){
		if( alive ) {
			x += xVel * frameTime;
			y += yVel * frameTime;
			sprite.setPosition( x, y );
		}
	}

	void setPosition( float xIn, float yIn ){
		x = xIn;
		y = yIn;
		sprite.setPosition( x, y );
	}

	void setTexture( sf::Texture &texture, int textureWidth ){
		sprite.setTexture(texture);
		diameter = textureWidth;
		radius = diameter / 2.f;
	}
};

}

#endif