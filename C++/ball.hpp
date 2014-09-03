#ifndef BALL_HPP
#define BALL_HPP

#include <SFML/Graphics.hpp>
#include "vector.hpp"

#define TEXTURE_DIA 10.f // Texture is current 10px in dia

namespace gp {

class Ball {
public:
	float x, y;
	float xVel, yVel;
	float diameter, radius;
	float springRate, reboundEfficiency;
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
			sprite.setPosition( x + radius, y + radius );
		}
	}

	void setPosition( float xIn, float yIn ){
		x = xIn;
		y = yIn;
		sprite.setPosition( x, y );
	}

	void setTexture( sf::Texture &texture, int ballDia, int r, int g, int b ){
		diameter = ballDia;
		radius = ballDia / 2.f;
		
		sprite.setOrigin(radius, radius);
		texture.setSmooth(true);
		sprite.setTexture(texture);
		sprite.setScale(diameter/TEXTURE_DIA, diameter/TEXTURE_DIA);
		sprite.setColor(sf::Color(r, g, b));

	}
};

}

#endif