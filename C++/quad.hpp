#ifndef QUAD_HPP
#define QUAD_HPP

#include <vector>
#include <cstddef>

//#include "particles.hpp"
//#include "ball.hpp"

namespace z {

class Particles;
class Ball;

class Quad {
//private:
public:

	Quad *parentQuad;
	Quad *childQuad[4];
//public:
	std::vector<z::Ball*> residentList;
	unsigned int level;
	unsigned int maxLevel;
	unsigned int childNum;
	double xMin;
	double xMax;
	double yMin;
	double yMax;
	
	static Particles *particles;
	
	Quad(Quad*, unsigned int, unsigned int, unsigned int, double, double, double, double);
	~Quad() {
		//for (int i = 0; i <= 3; i++) delete childQuad[i];
	}
	bool sortParticle(unsigned long int );
	void collideParticles(Ball*, bool);
	bool addParticle(Ball*, bool);
	bool trickleParticle(Ball*, bool);
	bool moveToGrandparent(Ball*);
	bool movetoParent(Ball*);
	bool moveToChild(unsigned int, Ball*);
};
}

#endif