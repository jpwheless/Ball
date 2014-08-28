#ifndef BALL_HPP
#define BALL_HPP

#include <SFML/Graphics.hpp>
#include "vector.hpp"

namespace gp {

class Ball {
public:
	Vector pos;
	Vector vel;
	float diameter;
	float radius;
	float springRate;
	sf::Sprite sprite;
	bool alive;

	void update( float frameTime ){
		if( alive ) {
			pos.x += vel.x * frameTime;
			pos.y += vel.y * frameTime;
		}
	}

	void setTexture( sf::Texture &texture, int textureWidth ){
		sprite.setTexture(texture);
		diameter = textureWidth;
		radius = diameter / 2.f;
	}
};

}

#endif