#ifndef QUAD_HPP
#define QUAD_HPP

#include <vector>
#include <cstddef>
#include <iostream>

#include "spinlock.hpp"

#define MAX_NULLS 10

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
	bool tooManyNulls;
	SpinLock writingLock;
		
	static Particles *particles;
	
	Quad(Quad*, unsigned int, unsigned int, unsigned int, double, double, double, double);
	bool sortParticle(Ball*);
	void cleanResidentList();
	void collideParticles(Ball*, bool);
	bool addParticle(Ball*, bool);
	bool checkIfResident(unsigned long int, bool);
	bool trickleParticle(Ball*, bool);
	bool moveToGrandparent(Ball*);
	bool movetoParent(Ball*);
	bool moveToChild(unsigned int, Ball*);
	void printParams();
};
}

#endif