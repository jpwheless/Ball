#ifndef VECTOR_HPP
#define VECTOR_HPP

namespace gp {

class Vector {
public:
	float x, y;

	Vector(){

	}
	
	Vector( float xIn, float yIn ) {
		x = xIn;
		y = yIn;
	}

	Vector& operator+=(const Vector &in){
		x += in.x;
		y += in.y;
		return *this;
	}

	Vector& operator-=(const Vector &in){
		x -= in.x;
		y -= in.y;
		return *this;
	}
};

}

#endif