#ifndef INPUT_HPP
#define INPUT_HPP

#define PI 3.14159265359
#define PI2 6.28318530718
#define PIovr2 1.57079632679
#define PIovr8 0.39269908169
#define PIovr4 0.78539816339
#define RAD_TICK 5
#define MAX_RAD 200
#define MIN_RAD 5
#define VEL_TICK 200
#define MAX_VEL 10000.0
#define MIN_VEL 1000.0
#define ANG_TICK 0.0872664626
#define VEL_LINE_SCALE 0.025
#define MAX_GRAV 400000
#define MIN_GRAV -400000
#define GRAV_TICK (MAX_GRAV-MIN_GRAV)/40.0
#define GRAV_LINE_SCALE 400.0/(MAX_GRAV-MIN_GRAV)
#define BHRAD_TICK 1
#define MAX_BHRAD 20
#define MIN_BHRAD 5

#include <SFML/Graphics.hpp>
#include "particles.hpp"

namespace z {

class Input {
public:
	z::Particles *particles;
	sf::RenderWindow *mainWindow;
	
	int *resX;
	int *resY;
	
	int mouseX;
	int mouseY;
	int mouseMode;
	int shootMode;
	int mouseRad;
	double shootAngClick;
	double shootAngDrag;
	double shootVelClick;
	double shootVelDrag;
	double bhRad;
	double bhGrav;
	
	double shootOrigin[2];
	
	int newBallDia, newBallDensity;
	
	InteractionSetting bhIinteract;
	
	bool windowFocused;
	bool mousePressed;
	bool mouseHeld;
	bool mouseReleased;
	bool modeChanged;
	bool freezeCircle;
	bool eraseFuncOnly;
	
	bool bhPermanent;
	bool paintOvr;
	bool paintForce;
	
	sf::Vertex shootLine[2];
	
	sf::CircleShape mouseCircle;
	
	Input(z::Particles *p) {
		particles = p;
		mouseMode = 1;
		shootMode = 1;
		mouseRad = MIN_RAD;
		shootAngClick = PIovr2;
		shootVelClick = MIN_VEL;
		bhGrav = particles->bhV[0].surfaceAccel;
		
		resX = particles->resX;
		resY = particles->resY;
		shootLine[0] = sf::Vertex(sf::Vector2f(mouseX, mouseY));
		shootLine[1] = sf::Vertex(sf::Vector2f(mouseX + VEL_LINE_SCALE*shootVelClick*cos(shootAngClick),
																					 mouseY + VEL_LINE_SCALE*shootVelClick*sin(shootAngClick)));
		mouseCircle.setRadius(mouseRad);
		mouseCircle.setOutlineColor(sf::Color::White);
		mouseCircle.setOutlineThickness(1);
		
		windowFocused = true; // Probably true, but no way to know
		mousePressed = false;
		mouseHeld = false;
		mouseReleased = false;
		modeChanged = false;
		freezeCircle = false;
		eraseFuncOnly = false;
		
		bhPermanent = false;
		paintOvr = false;
		paintForce = false;
		
		bhIinteract = COLLISION;
		bhRad = particles->bhV[0].radius;
	}
	
	void update() {
		if (windowFocused) {
			if (mousePressed) mouseHeld = true;
			else if (mouseReleased) mouseHeld = false;
			sf::Vector2i mousePos = sf::Mouse::getPosition(*mainWindow);
			mouseX = mousePos.x;
			mouseY = mousePos.y;
			
			if (modeChanged) {
				freezeCircle = false;
				particles->mobilizeCloud();
			}
			
			switch(mouseMode) {
				case 1: // Erase
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
						if (eraseFuncOnly) particles->deactivateCloud(mouseX, mouseY, mouseRad, newBallDia, newBallDensity);
						else particles->deactivateCloud(mouseX, mouseY, mouseRad);
					}
					break;
				case 2: // Drag
					if (mousePressed) {
						particles->mobilizeCloud();
						particles->immobilizeCloud(mouseX, mouseY, mouseRad);
					}
					else if (mouseHeld) {
						particles->moveCloud(mouseX, mouseY);
					}
					else if (mouseReleased) {
						particles->mobilizeCloud();
					}
					break;
				case 3: // Paint
					if (paintOvr) {
						if (mouseReleased && mouseX <= *resX) {
							particles->deactivateCloud(mouseX, mouseY, mouseRad);
							particles->createCloud(mouseX, mouseY, mouseRad, 0, 0, newBallDia, newBallDensity, false, true);
						}
					}
					else if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseX <= *resX) {
						particles->createCloud(mouseX, mouseY, mouseRad, 0, 0, newBallDia, newBallDensity, false, paintForce);
					}
					break;
				case 4: // Shoot
					switch (shootMode) {
						case 1:
							if (mouseReleased && mouseX <= *resX) {
								particles->deactivateCloud(mouseX, mouseY, mouseRad);
								particles->createCloud(mouseX, mouseY, mouseRad, shootVelClick, shootAngClick, newBallDia, newBallDensity, false, false);
							}
							break;
						case 2:
							if (mousePressed && mouseX <= *resX) {
								freezeCircle = true;
								shootOrigin[0] = mouseX;
								shootOrigin[1] = mouseY;
							}
							if (mouseHeld && freezeCircle) {
								shootVelDrag = sqrt(pow(shootOrigin[0]-mouseX, 2.0) + pow(shootOrigin[1]-mouseY, 2.0))/(2.0*VEL_LINE_SCALE);
								shootVelDrag = (shootVelDrag < MIN_VEL)?(MIN_VEL):((shootVelDrag > MAX_VEL)?MAX_VEL:shootVelDrag);
								if (shootOrigin[0] == mouseX) {
									if (shootOrigin[1] == mouseY) shootAngDrag = 0.0;
									else if (shootOrigin[1] > mouseY) shootAngDrag =  PI + PIovr2;
									else if (shootOrigin[1] < mouseY) shootAngDrag = PIovr2;
								}
								else if (shootOrigin[0] > mouseX)
									shootAngDrag = PI + atan((shootOrigin[1]-mouseY)/(shootOrigin[0]-mouseX));
								else if (shootOrigin[0] < mouseX)
									shootAngDrag = atan((shootOrigin[1]-mouseY)/(shootOrigin[0]-mouseX));
							}
							else if (mouseReleased && freezeCircle) {
								particles->deactivateCloud(shootOrigin[0], shootOrigin[1], mouseRad);
								particles->createCloud(shootOrigin[0], shootOrigin[1], mouseRad, shootVelDrag, shootAngDrag, newBallDia, newBallDensity, false, false);
								freezeCircle = false;
							}
							break;
					}
					break;
				case 5: // Place Blackhole
					if (mouseReleased && mouseX <= *resX) {
						particles->createBH(mouseX, mouseY, bhGrav, bhRad*2.0, bhIinteract);
					}
					break;
				case 6: // Control Blackhole
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseX <= *resX) {
						if (mousePressed && !bhPermanent) {
							particles->bhV[0].setPosition(mouseX, mouseY);
							particles->bhV[0].active = true;
						}
						else {
							particles->bhV[0].xMove = mouseX;
							particles->bhV[0].yMove = mouseY;
						}
					}
					else if (mouseReleased) {
						if (!bhPermanent) particles->bhV[0].active = false;
						else {
							particles->bhV[0].xMove = *resX/2.0;
							particles->bhV[0].yMove = *resY/2.0;
						}
					}
					break;
				default:
					break;
			}
		}
		// Reset flags
		mousePressed = false;
		mouseReleased = false;
		modeChanged = false;
	}
		
	// Handle mouse wheel events
	void inputWheel(sf::Event &event) {
		if (windowFocused) {
			switch(mouseMode) {
				case 2:
					if (mouseHeld) break;
				case 1:
				case 3:
					mouseRad += event.mouseWheel.delta*RAD_TICK;
					mouseRad = (mouseRad < MIN_RAD)?(MIN_RAD):((mouseRad > MAX_RAD)?MAX_RAD:mouseRad);
					break;
				case 4:
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || 
							sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
						if (shootMode == 1) {
							shootAngClick -= event.mouseWheel.delta*ANG_TICK;
							shootAngClick = (shootAngClick < 0.0)?(PI2):((shootAngClick > PI2)?0.0:shootAngClick);
						}
					}
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || 
							sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
						if (shootMode == 1) {
							shootVelClick += event.mouseWheel.delta*VEL_TICK;
							shootVelClick = (shootVelClick < MIN_VEL)?(MIN_VEL):((shootVelClick > MAX_VEL)?MAX_VEL:shootVelClick);
						}
					}
					else {
						mouseRad += event.mouseWheel.delta*RAD_TICK;
						mouseRad = (mouseRad < MIN_RAD)?(MIN_RAD):((mouseRad > MAX_RAD)?MAX_RAD:mouseRad);
					}
					break;
				case 5:
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || 
							sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
						bhRad += event.mouseWheel.delta*BHRAD_TICK;
						bhRad = (bhRad < MIN_BHRAD)?(MIN_BHRAD):((bhRad > MAX_BHRAD)?MAX_BHRAD:bhRad);
					}
					else {
						bhGrav -= event.mouseWheel.delta*GRAV_TICK;
						bhGrav = (bhGrav < MIN_GRAV)?(MIN_GRAV):((bhGrav > MAX_GRAV)?MAX_GRAV:bhGrav);
					}
					break;
				case 6:
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || 
							sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
						particles->bhV[0].radius += event.mouseWheel.delta*BHRAD_TICK;
						particles->bhV[0].radius = (particles->bhV[0].radius < MIN_BHRAD)?(MIN_BHRAD):
								((particles->bhV[0].radius > MAX_BHRAD)?MAX_BHRAD:particles->bhV[0].radius);
						particles->bhV[0].setSize(particles->bhV[0].radius*2.0);
					}
					else {
						particles->bhV[0].surfaceAccel -= event.mouseWheel.delta*GRAV_TICK;
						particles->bhV[0].surfaceAccel = (particles->bhV[0].surfaceAccel < MIN_GRAV)?(MIN_GRAV):
								((particles->bhV[0].surfaceAccel > MAX_GRAV)?MAX_GRAV:particles->bhV[0].surfaceAccel);
						particles->bhV[0].setAttraction(particles->bhV[0].surfaceAccel);
					}
					break;
				default:
					break;
			}
		}
	}
	
	void draw() {
		if (windowFocused) {
			switch(mouseMode) {
				case 1:
				case 2:
				case 3:
					mouseCircle.setRadius(mouseRad);
					mouseCircle.setPosition(mouseX-mouseRad, mouseY-mouseRad);
					mouseCircle.setFillColor(sf::Color::Transparent);
					mainWindow->draw(mouseCircle);
					break;
				case 4:
					switch (shootMode) {
						case 1:
							mouseCircle.setRadius(mouseRad);
							mouseCircle.setPosition(mouseX-mouseRad, mouseY-mouseRad);
							mouseCircle.setFillColor(sf::Color::Transparent);
							shootLine[0] = sf::Vertex(sf::Vector2f(mouseX, mouseY));
							shootLine[1] = sf::Vertex(sf::Vector2f(mouseX + VEL_LINE_SCALE*shootVelClick*cos(shootAngClick),
																										 mouseY + VEL_LINE_SCALE*shootVelClick*sin(shootAngClick)));
							mainWindow->draw(mouseCircle);
							mainWindow->draw(shootLine, 2, sf::Lines);
							break;
						case 2:
							if (freezeCircle) {
								mouseCircle.setRadius(mouseRad);
								mouseCircle.setPosition(shootOrigin[0]-mouseRad, shootOrigin[1]-mouseRad);
								mouseCircle.setFillColor(sf::Color::Transparent);
								
								shootLine[0] = sf::Vertex(sf::Vector2f(shootOrigin[0], shootOrigin[1]));
								shootLine[1] = sf::Vertex(sf::Vector2f(shootOrigin[0] + 2.0*VEL_LINE_SCALE*shootVelDrag*cos(shootAngDrag),
																											 shootOrigin[1] + 2.0*VEL_LINE_SCALE*shootVelDrag*sin(shootAngDrag)));
								mainWindow->draw(mouseCircle);
								mainWindow->draw(shootLine, 2, sf::Lines);
							}
							else {
								mouseCircle.setRadius(mouseRad);
								mouseCircle.setPosition(mouseX-mouseRad, mouseY-mouseRad);
								mouseCircle.setFillColor(sf::Color::Transparent);
								mainWindow->draw(mouseCircle);
							}
							break;
					}
					break;
				case 5:
					mouseCircle.setRadius(bhRad);
					mouseCircle.setRadius(bhRad);
					if (bhGrav < 0) mouseCircle.setFillColor(sf::Color(255, 255, 255));
					else if (bhGrav == 0) mouseCircle.setFillColor(sf::Color(127, 127, 127));
					else mouseCircle.setFillColor(sf::Color(0, 0, 0)); 
					mouseCircle.setPosition(mouseX-bhRad, mouseY-bhRad);
					mainWindow->draw(mouseCircle);
					shootLine[0] = sf::Vertex(sf::Vector2f(mouseX, mouseY));
					for (double ang = 0; ang <= PI2; ang += PIovr4) {
						shootLine[1] = sf::Vertex(sf::Vector2f(mouseX + GRAV_LINE_SCALE*bhGrav*cos(ang),
																									 mouseY + GRAV_LINE_SCALE*bhGrav*sin(ang)));
						mainWindow->draw(shootLine, 2, sf::Lines);
					}
					break;
				case 6:
					if (particles->bhV[0].active && (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || 
							sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))) {
						shootLine[0] = sf::Vertex(sf::Vector2f(particles->bhV[0].x, particles->bhV[0].y));
						for (double ang = 0; ang <= PI2; ang += PIovr4) {
							shootLine[1] = sf::Vertex(sf::Vector2f(particles->bhV[0].x + GRAV_LINE_SCALE*particles->bhV[0].surfaceAccel*cos(ang),
																										 particles->bhV[0].y + GRAV_LINE_SCALE*particles->bhV[0].surfaceAccel*sin(ang)));
							mainWindow->draw(shootLine, 2, sf::Lines);
						}
					}
					break;
				default:
					break;
			}
		}
	}
	
};

}

#endif