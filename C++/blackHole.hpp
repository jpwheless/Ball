#ifndef BLACK_HOLE_HPP
#define BLACK_HOLE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

namespace z {

enum InteractionSetting {
	NO_COLLISION,
	DESTRUCTION,
	COLLISION
};

enum MouseSetting {
	STATIONARY,
	MOUSE_CONTROL,
	PERM_AND_MOUSE
};

class BlackHole {
public:
	float x, y, xPerm, yPerm, surfaceAccel, centerAccel;
	int diameter;
	float mouseFilter;
	InteractionSetting interact;
	MouseSetting mouseSetting;
	bool leftMousePressed;

	BlackHole( float xPerm, float yPerm, float surfaceAccel, int diameter, InteractionSetting interact, MouseSetting mouseSetting ) {
		this->xPerm = xPerm;
		this->yPerm = yPerm;
		this->surfaceAccel = surfaceAccel;
		this->centerAccel = surfaceAccel * pow( diameter / 2.f , 2);
		this->diameter = diameter;
		this->interact = interact;
		this->mouseSetting = mouseSetting;
		mouseFilter = 0.05;
	}

	void processInput( int mouseX, int mouseY, bool leftMousePressed ) {
		this->leftMousePressed = leftMousePressed;
		if( mouseSetting != STATIONARY ) {
			if( leftMousePressed ) {
				x = mouseFilter * xPerm + ( 1.f - mouseFilter ) * x;
				y = mouseFilter * yPerm + ( 1.f - mouseFilter ) * y; 
			}
		}	
	}

	void processBalls( std::vector<Ball>& ball, float frameTime ) {
		if ( interact == COLLISION ) {
			for ( int i = 0; i < ball.size(); i++ ){
				if(ball[i].alive){
					float radius = sqrt( pow(ball[i].x - x, 2 ) + pow( ball[i].y - y, 2 ));
					if ( radius < ( ball[i].diameter / 2.f) ) { 
						float term = ball[i].springRate * ((ball[i].diameter + diameter) / 2.f - radius ) * frameTime;
						
						ball[i].xVel += (( ball[i].x - x ) / radius ) * term;
						ball[i].yVel += (( ball[i].y - y ) / radius ) * term;
					}
				}
			}
		}

		if( mouseSetting == STATIONARY || mouseSetting == PERM_AND_MOUSE || ( mouseSetting == MOUSE_CONTROL && leftMousePressed )) {
			for ( int i = 0; i < ball.size(); i++ ){
				float term;
				if( ball[i].alive ) {
					float radius = sqrt( pow(ball[i].x - x, 2) + pow( ball[i].y - y, 2) );

					if ( radius < ( diameter / 2.f )) {
						term = (centerAccel / pow(radius, 2)) *  frameTime;
					} else {
						if ( interact = DESTRUCTION ){
							ball[i].alive = false;
						} else {
							term = surfaceAccel * frameTime;
						}
					}

					ball[i].xVel += (( x - ball[i].x ) / radius) * term;
					ball[i].yVel += (( y - ball[i].y ) / radius) * term;
				}
			}
		}
	}
};
}

#endif