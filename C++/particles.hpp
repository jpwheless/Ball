#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include <cmath>
#include <vector>

#include "blackHole.hpp"
#include "quad.hpp"
#include "ball.hpp"

#define NUM_TRIES 25
#define PI 3.14159265359
#define PI2 6.28318530718
#define PI60 1.04719755

#define MAX_PARTICLES 10000
#define MAX_BH 1000
#define PARTICLE_CLEAN 500
#define BH_CLEAN 10

#define LEVELS 4

namespace z {

class Particles {
//private:
public:
	Quad* quadTree;

//public:
	int *resX, *resY;
	double *tickTime;
	double linGravity;
	bool particleCollisions;
	bool particleStickyness;
	bool boundCeiling;
	bool boundWalls;
	bool boundFloor;
		
	double prevX;
	double prevY;
	
	std::vector<z::Ball*> ballV;
	std::vector<z::BlackHole> bhV;
	
	std::vector<int> listParticles;
	std::vector<int> listBH;
	
	int ballAlive;
	int bhAlive;
	
	double maxParticleVel;
	
	/////////////////
	// Constructor //
	/////////////////
	Particles(int *resXT, int *resYT, double *tickTimeT, double linGravityT);
	~Particles() {
		for (unsigned int k = 0; k < ballV.size(); k++) delete ballV[k];
		delete quadTree;
	}
	inline double randDouble(double minimum, double maximum) {
		double r = (double)rand()/(double)RAND_MAX;
		return minimum + r*(maximum - minimum);
	}
	
	///////////////////////
	// Particle Creation //
	///////////////////////
	void createInitBalls(unsigned int, int, int);
	void createCloud(double, double, double, double, double, int, int, bool, bool);
	int createParticle(double, double, double, double, int, int, bool, bool);
	///////////////////////////
	// Particle Manipulation //
	///////////////////////////
	void createBH(int, int, double, int, InteractionSetting);
	inline int size() {
		return ballV.size();
	}
	void cleanParticles();
	void cleanBH();
	void cleanQuad();
	void zeroVel();
	void clearParticles();
	void immobilizeCloud(double, double, double);
	void moveCloud(double, double);
	void mobilizeCloud();
	void deactivateCloud(double, double, double);
	
	/////////////
	// Physics //
	/////////////
	void quadSortParticles(unsigned int, unsigned int);
	void quadCollideParticles(unsigned int, unsigned int);
	void addPhysics(unsigned int, unsigned int);
	
	// Assumes that particleCollisions and both balls are alive
	void collisonUpdate(Ball*, Ball*);
	void draw(sf::RenderWindow*);
};

}

#endif