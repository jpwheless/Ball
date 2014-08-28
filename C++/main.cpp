#include <SFML/Graphics.hpp>
#include <vector>
#include <stdlib.h>
#include <iostream>

#include "ball.hpp"
#include "vector.hpp"
#include "blackHole.hpp"

#define RESX 1200
#define RESY 720
#define X_GRAV 0.f
#define Y_GRAV 300.f
#define WALL_REB_EFF 0.5f
#define NUM_OF_BALLS 50

void collisonUpdate( std::vector<gp::Ball> &balls, float frameTime );
void applyGravity( std::vector<gp::Ball> &balls, float frameTime );

int main()
{
	sf::RenderWindow window( sf::VideoMode( RESX, RESY ), "Balls!");
	window.setFramerateLimit(60);
	sf::Clock timer;
	sf::Time elapsed;
	float frameTime;
	sf::Texture bgTexture;
	bgTexture.loadFromFile( "textures/bg.png" );
	sf::Sprite bg;
	bg.setTexture(bgTexture);

	sf::Texture ballTexture;
	ballTexture.loadFromFile("textures/ball.png");
	std::vector<gp::Ball> balls;
	for ( int i = 0; i < NUM_OF_BALLS; i++ ) {
		gp::Ball ball;
		ball.setPosition(rand()%RESX, rand()%RESY);
		ball.sprite.setTexture(ballTexture);
		ball.vel = gp::Vector( 0, 0 );
		balls.push_back(ball);
	}

	gp::BlackHole blackHole( RESX / 2.f, RESY / 2.f, 100000.f, 100, gp::NO_COLLISION, gp::STATIONARY );

	while (window.isOpen())
	{
		elapsed = timer.restart();
		frameTime = elapsed.asSeconds();


		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		blackHole.processBalls( balls, frameTime );
		collisonUpdate( balls, frameTime );
		applyGravity ( balls, frameTime );

		for ( int i = 0; i < balls.size(); i++ )
			balls[i].update( frameTime );

		window.clear();
		window.draw(bg);
		std::cout << balls[0].pos.x << std::endl;
		for ( int i = 0; i < balls.size(); i++ ){
			window.draw(balls[i].sprite);
		}
		window.display();
	}

	return 0;
}

// Rebound efficiency not complete
void collisonUpdate( std::vector<gp::Ball> &ball, float frameTime ){
	int size = ball.size();
	for (int i = 0; i < size; i++) {
		if ( ball[i].alive ) {

			for (int j = i+1; j < size; j++) {
				if ( ball[j].alive ) {
					float distance = sqrt( pow(ball[i].pos.x - ball[j].pos.x, 2 ) + pow( ball[i].pos.y - ball[j].pos.y, 2 ));
					float neutralDistance = ball[i].radius + ball[j].radius;
					if (distance < neutralDistance) { // Particles are colliding
						float springTerm = (ball[i].springRate*( neutralDistance - distance ) / (2.f*frameTime));
						float xDir = ((ball[j].pos.x - ball[i].pos.x) / distance)
						float yDir = ((ball[j].pos.y - ball[i].pos.y) / distance)

						ball[j].vel.x += xDir * springTerm * 
							(ball[j].vel.x - ball[i].vel.x > 0.f) ? ball[j].reboundEfficiency : 1.f;
						ball[i].vel.x -= xDir * springTerm * 
							(ball[j].vel.x - ball[i].vel.x > 0.f) ? ball[i].reboundEfficiency : 1.f;
						ball[j].vel.y += yDir * springTerm * 
							(ball[j].vel.y - ball[i].vel.y > 0.f) ? ball[j].reboundEfficiency : 1.f;
						ball[i].vel.y -= yDir * springTerm * 
							(ball[j].vel.y - ball[i].vel.y > 0.f) ? ball[i].reboundEfficiency : 1.f;
					}
				} 
			}
		}
	}
}

void applyGravity( std::vector<gp::Ball> &ball, float frameTime ){
	for (int i = 0; i < ball.size(); i++) {
		if ( ball[i].alive ) {
		
			if (ball[i].pos.x + ball[i].radius > RESX ) {
				ball[i].vel.x += (RESX - (ball[i].pos.x + ball[i].radius)) * ball[i].springRate *
					((ball[i].vel.x < 0) ? WALL_REB_EFF : 1.0) * frameTime;
			}
			else if (ball[i].pos.x - ball[i].radius < 0) {
				ball[i].vel.x += (ball[i].pos.x - ball[i].radius) * ball[i].springRate *
					((ball[i].vel.x > 0) ? WALL_REB_EFF : 1.0) * frameTime;
			}
			else {
				ball[i].vel.x += X_GRAV* frameTime;
			}
			
			if (ball[i].pos.y + ball[i].radius  > RESY ) {
				ball[i].vel.y += (RESY - (ball[i].pos.y + ball[i].radius)) * ball[i].springRate *
					((ball[i].vel.y < 0) ? WALL_REB_EFF : 1.0) * frameTime;
			}
			else if (ball[i].pos.y - ball[i].radius < 0) {
				ball[i].vel.y += (ball[i].pos.y - ball[i].radius)*ball[i].springRate *
					((ball[i].vel.y > 0) ? WALL_REB_EFF : 1.0) * frameTime;
			}
			else {
				ball[i].vel.y += Y_GRAV* frameTime;
			}
		}
	}
}