#include "quad.hpp"
#include "ball.hpp"
#include "particles.hpp"

namespace z {
	
	Quad::Quad(Quad *parentQ, unsigned int thisLevel, unsigned int maxLevel, unsigned int childNum, double xMin, double xMax, double yMin, double yMax) {
		this->xMin = xMin;
		this->xMax = xMax;
		this->yMin = yMin;
		this->yMax = yMax;
		this->maxLevel = maxLevel;
		
		level = thisLevel;
		
		if (thisLevel == 0) {
			parentQuad = NULL;
			childNum = 0;
		}
		else	parentQuad = parentQ;
		
		if (thisLevel < maxLevel) {
			double xRange = (xMax - xMin)/2.0;
			double yRange = (yMax - yMin)/2.0;
			// Assuming positive y is down, positive x is right
			// Top left
			childQuad[0] = new Quad(this, level + 1, maxLevel, 0, xMin, xMin+xRange, yMin, yMin+yRange);
			// Top right
			childQuad[1] = new Quad(this, level + 1, maxLevel, 1, xMin+xRange, xMax, yMin, yMin+yRange);
			// Bottom left
			childQuad[2] = new Quad(this, level + 1, maxLevel, 2, xMin, xMin+xRange, yMin+yRange, yMax);
			// Bottom right
			childQuad[3] = new Quad(this, level + 1, maxLevel, 3, xMin+xRange, xMax, yMin+yRange, yMax);
		}
	}
		
	// Pass unique ID of particle that resides in this quad
	// Particle will be moved to correct location in tree
	bool Quad::sortParticle(unsigned long int pID) {
		bool found = false;
		int i;
		for (i = 0; i < residentList.size() && !found; i++) {
			if (residentList[i]->getID() == pID) {
				if (trickleParticle(residentList[i], true)) { // Return true if particle is moved
					// This function may take some time to return
					// Another thread may have reordered this residentList
					// Make sure it's the right particle
					if (residentList[i]->getID() == pID) residentList.erase(residentList.begin()+i);
					else {
						for (i = 0; i < residentList.size() && !found; i++) {
							if (residentList[i]->getID() == pID) residentList.erase(residentList.begin()+i);
						}
					}
				}
			}
		}
		return found;
	}
	
	// Search for particle collisions in all particles lower in the tree than passed particle
	void Quad::collideParticles(Ball *particleA, bool resident) {
		bool found = !resident;
		if (resident) { // Find particle in resident list
			int i;
			for (i = 0; i < residentList.size() && !found; i++) {
				if (residentList[i]->getID() == particleA->getID()) {
					found = true;
					i++;
					// Collide all particles under it
					for (; i < residentList.size(); i++) {
						if (residentList[i]->alive) particles->collisonUpdate(particleA, residentList[i]);
					}
				}
			}
		}
		else {
			for (int i = 0; i < residentList.size(); i++) {
				if (residentList[i]->alive) particles->collisonUpdate(particleA, residentList[i]);
			}
		}
		// Do not collide children if particle was supposed to be found and wasn't
		if (found && level < maxLevel) {
			for (int i = 0; i <= 3; i++) {
				childQuad[i]->collideParticles(particleA, false);
			}
		}
	}
		
	bool Quad::addParticle(Ball *movedParticle, bool checkBounds) {
		// if sortedDown, bounds have been checked by parent
		// if trickleParticle returns false, particle must be added to residents
		if (!trickleParticle(movedParticle, checkBounds)) {
			residentList.push_back(movedParticle);
			movedParticle->quadResidence = this;
		}
		return true; // This can't not work, I guess
	}
	
	// Checks bounds and passes to correct Quad if necessary
	// Return true if particle is moved
	bool Quad::trickleParticle(Ball *newParticle, bool checkBounds) {
		double boundArray[4]; // xMin, xMax, yMin, yMax
		
		newParticle->getBounds(boundArray);
		
		// Check if out of bounds
		// Move to parent if so
		if (checkBounds && level != 0) {
			switch (childNum) {
				case 0: // Top left
					if (boundArray[0] < xMin || boundArray[2] < yMin) {
						return moveToGrandparent(newParticle);
					}
					else if (boundArray[1] > xMax || boundArray[3] > yMax) {
						return movetoParent(newParticle);
					}
					break;
				case 1: // Top right
					if (boundArray[0] < xMin || boundArray[3] > yMax) {
						return moveToGrandparent(newParticle);
					}
					else if (boundArray[1] > xMax || boundArray[2] < yMin) {
						return movetoParent(newParticle);
					}
					break;
				case 2: // Bottom left	
					if (boundArray[0] < xMin || boundArray[3] > yMax) {
						return moveToGrandparent(newParticle);
					}
					else if (boundArray[1] > xMax || boundArray[2] < yMin) {
						return movetoParent(newParticle);
					}
					break;
				case 3: // Bottom right
					if (boundArray[1] > xMax || boundArray[3] > yMax) {
						return moveToGrandparent(newParticle);
					}
					else if (boundArray[0] < xMin || boundArray[2] < yMin) {
						return movetoParent(newParticle);
					}
					break;
			}
		}
		// Particle is within bounds
		// See if it needs moving to child
		if (level < maxLevel) {
			double xMid = xMin + (xMax - xMin)/2.0;
			double yMid = yMin + (yMax - yMin)/2.0;
			if (boundArray[1] < xMid) { // Left
				if (boundArray[4] < yMid) { // Top
					return moveToChild(0, newParticle);
				}
				else if (boundArray[3] > yMid) { // Bottom
					return moveToChild(2, newParticle);
				}
			}
			else if (boundArray[0] > xMid) { // Right
				if (boundArray[4] < yMid) { // Top
					return moveToChild(1, newParticle);
				}
				else if (boundArray[3] > yMid) { // Bottom
					return moveToChild(3, newParticle);
				}
			}
		}
		// Particle cannot be moved anywhere else
		return false;
	}
	
	bool Quad::moveToGrandparent(Ball *movedParticle) {
		if (level > 1) return parentQuad->movetoParent(movedParticle);
		else return parentQuad->addParticle(movedParticle, true);
	}
	
	bool Quad::movetoParent(Ball *movedParticle) {
		if (level > 0) {
			return parentQuad->addParticle(movedParticle, true);
		}
		else return false;
	}
	
	bool Quad::moveToChild(unsigned int childNum, Ball *movedParticle) {
		return childQuad[childNum]->addParticle(movedParticle, false);
	}
	
	Particles *Quad::particles;
	
}