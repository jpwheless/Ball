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
			this->childNum = 0;
		}
		else	{
			parentQuad = parentQ;
			this->childNum = childNum;
		}
		
		residentList.reserve(MAX_PARTICLES);
		
		tooManyNulls = false;
				
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
		else for (int i = 0; i <= 3; i++) childQuad[i] = NULL;
	}
		
	// Pass unique ID of particle that resides in this quad
	// Particle will be moved to correct location in tree
	bool Quad::sortParticle(Ball* sortedParticle) {
		bool found = false;
		unsigned int pID = sortedParticle->getID();
		unsigned int i;
		for (i = 0; i < residentList.size() && !found; i++) {
			if (residentList[i] != NULL && residentList[i]->getID() == pID) {
				found = true;
				if (trickleParticle(sortedParticle, true)) { // Return true if particle is moved
					// This function may take some time to return
					// Another thread may have reordered this residentList
					// Make sure it's the right particle
					if (residentList[i]->getID() == pID) 
						residentList[i] = NULL;
					else {
						found = false;
						for (i = 0; i < residentList.size() && !found; i++) {
							if (residentList[i] != NULL && residentList[i]->getID() == pID) 
								residentList[i] = NULL;
						}
					}
				}
			}
		}
		return found;
	}
	
	void Quad::cleanResidentList() {
		if (tooManyNulls) {
			int frontSwap = 0;
			int backSwap = residentList.size() - 1;
			while (frontSwap < backSwap) {
				while (frontSwap < residentList.size() && residentList[frontSwap] != NULL) frontSwap++; // Find dead ball
				while (backSwap > 0 && residentList[backSwap] == NULL) {
					backSwap--; // Find live ball
					//std::cout << residentList.size() << "\t" << backSwap << "\n";
				}
				if (frontSwap < backSwap) std::swap(residentList[frontSwap],residentList[backSwap]); // Swap
			}
			
			backSwap = residentList.size();
			while (backSwap > 0) {
				if (residentList[backSwap-1] == NULL) backSwap--;
				else break;
			}
			if (backSwap < residentList.size()) {
				//int eraseStart = (backSwap < 50)?50:backSwap;
				residentList.erase(residentList.begin()+backSwap, residentList.end());
			}
		}
		if (level < maxLevel) for (int i = 0; i <= 3; i++) childQuad[i]->cleanResidentList();
	}
		
	bool Quad::addParticle(Ball *movedParticle, bool checkBounds) {
		// if trickleParticle returns false, particle must be added to residents
		if (!trickleParticle(movedParticle, checkBounds)) {
			unsigned int i = 0;
			bool nullEntry = false;
			writingLock.lock();
			for (; i < residentList.size() && !nullEntry; i++) {
				if (residentList[i] == NULL) {
					nullEntry = true;
					break;
				}
			}
			if (nullEntry) {
				residentList[i] = movedParticle;
			}
			else {
				residentList.push_back(movedParticle);
			}
			writingLock.unlock();
			
			movedParticle->quadResidence = this;
		}
		return true; // This can't not work, I guess
	}
	
	// Search for particle collisions in all particles lower in the tree than passed particle
	// Also does double-duty counting number of NULLs in residentList
	void Quad::collideParticles(Ball *particleA, bool resident) {
		bool found = !resident;
		unsigned int nullCount = 0;
		if (resident) { // Find particle in resident list
			unsigned int i;
			for (i = 0; i < residentList.size() && !found; i++) {
				if (residentList[i] != NULL && residentList[i]->getID() == particleA->getID()) {
					found = true;
					i++;
					// Collide all particles under it
					for (; i < residentList.size(); i++) {
						if (residentList[i] != NULL && residentList[i]->alive) 
							particles->collisonUpdate(particleA, residentList[i]);
					}
				}
				else nullCount++;
			}
		}
		else {
			for (unsigned int i = 0; i < residentList.size(); i++) {
				if (residentList[i] != NULL && residentList[i]->alive)
					particles->collisonUpdate(particleA, residentList[i]);
			}
		}
		// Do not collide children if particle was supposed to be found and wasn't
		if (found && level < maxLevel) {
			for (unsigned int i = 0; i <= 3; i++) {
				childQuad[i]->collideParticles(particleA, false);
			}
		}
		if (nullCount > MAX_NULLS) tooManyNulls = true;
	}
	
	bool Quad::checkIfResident(unsigned long int pID, bool deleteResident) {
		bool found = false;
		unsigned int i;
		for (i = 0; i < residentList.size() && !found; i++) {
			if (residentList[i] != NULL && residentList[i]->getID() == pID) {
				found = true;
				if (deleteResident) residentList[i] = NULL;
			}
		}
		return found;
	}

	// Checks bounds and passes to correct Quad if necessary
	// Return true if particle is moved
	bool Quad::trickleParticle(Ball *movingParticle, bool checkBounds) {
		movingParticle->updateBounds();
		// Check if out of bounds
		// Move to parent if so
		if (checkBounds && level > 0) {
			switch (childNum) {
				case 0: // Top left
					if (movingParticle->xMin < xMin || movingParticle->yMin < yMin) {
						return moveToGrandparent(movingParticle);
					}
					else if (movingParticle->xMax > xMax || movingParticle->yMax > yMax) {
						return movetoParent(movingParticle);
					}
					break;
				case 1: // Top right
					if (movingParticle->xMin < xMin || movingParticle->yMax > yMax) {
						return moveToGrandparent(movingParticle);
					}
					else if (movingParticle->xMax > xMax || movingParticle->yMin < yMin) {
						return movetoParent(movingParticle);
					}
					break;
				case 2: // Bottom left	
					if (movingParticle->xMin < xMin || movingParticle->yMax > yMax) {
						return moveToGrandparent(movingParticle);
					}
					else if (movingParticle->xMax > xMax || movingParticle->yMin < yMin) {
						return movetoParent(movingParticle);
					}
					break;
				case 3: // Bottom right
					if (movingParticle->xMax > xMax || movingParticle->yMax > yMax) {
						return moveToGrandparent(movingParticle);
					}
					else if (movingParticle->xMin < xMin || movingParticle->yMin < yMin) {
						return movetoParent(movingParticle);
					}
					break;
			}
		}
		if (level < maxLevel) {  // Particle is within bounds - See if it needs moving to child
			double xMid = xMin + (xMax - xMin)/2.0;
			double yMid = yMin + (yMax - yMin)/2.0;
			if (movingParticle->yMax < yMid) { // Top
				if (movingParticle->xMax < xMid) { // Left
					return moveToChild(0, movingParticle);
				}
				else if (movingParticle->xMin > xMid) { // Right
					return moveToChild(1, movingParticle);
				}
			}
			else if (movingParticle->yMin > yMid) { // Bottom
				if (movingParticle->xMax < xMid) { // Left
					return moveToChild(2, movingParticle);
				}
				else if (movingParticle->xMin > xMid) { // Right
					return moveToChild(3, movingParticle);
				}
			}
		}
		// Particle cannot be moved anywhere else		
		
		/*
		std::cout << "\n";
		std::cout << "ID: " << newParticle->getID() << ", Level: " << level << ", ChildNum: " << childNum << "\n";
		std::cout << "Quad xMin, xMax, yMin, yMax: " << xMin << "\t" << xMax << "\t" << yMin << "\t" << yMax << "\n";
		std::cout << "Ball xMin, xMax, yMin, yMax: " << boundArray[0] << "\t" << movingParticle->xMax << "\t" << movingParticle->yMin << "\t" << movingParticle->yMax << "\n";
		std::cout << "\n";
		*/
		
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
	
	void Quad::printParams() {
		std::cout << "Level: " << level << ", Child: " << childNum;
		std::cout << ", xMin, xMax, yMin, yMax: " << xMin << "\t" << xMax << "\t" << yMin << "\t" << yMax << "\n";
		std::cout << "\tResidents: \n";
		for (unsigned int i = 0; i < residentList.size(); i++) {
			if (residentList[i] != NULL) std::cout << "\t\t" << residentList[i]->getID() << "\n";
			else std::cout << "\t\tNULL\n";
		}
		
		if (level < maxLevel) {
			childQuad[0]->printParams();
			childQuad[1]->printParams();
			childQuad[2]->printParams();
			childQuad[3]->printParams();
		}
	}
	
	Particles *Quad::particles;
	
}