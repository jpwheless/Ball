#define PI 3.14159265359
#define PI2 6.28318530718
#define PIovr2 1.57079632679
#define PIovr8 0.39269908169
#define PIovr4 0.78539816339
#define RAD_TICK 5
#define MAX_RAD 100
#define MIN_RAD 5
#define VEL_TICK 200
#define MAX_VEL 8000
#define MIN_VEL 1000
#define ANG_TICK 0.0872664626
#define VEL_LINE_SCALE 0.025f
#define MAX_GRAV 400000
#define MIN_GRAV -400000
#define GRAV_TICK (MAX_GRAV-MIN_GRAV)/40.f
#define GRAV_LINE_SCALE 400.f/(MAX_GRAV-MIN_GRAV)
#define BHRAD_TICK 1
#define MAX_BHRAD 20
#define MIN_BHRAD 5


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
	int mouseRad;
	double shootAng;
	double shootVel;
	double bhRad;
	double bhGrav;
	
	InteractionSetting bhIinteract;
	
	bool windowFocused;
	bool mousePressed;
	bool mouseHeld;
	bool mouseReleased;
	bool modeChanged;
	
	bool bhPermanent;
	bool paintOvr;
	bool paintForce;
	
	sf::Vertex shootLine[2];
	
	sf::CircleShape mouseCircle;
	
	Input(z::Particles *p, sf::RenderWindow *mW) {
		particles = p;
		mainWindow = mW;
		mouseMode = 1;
		mouseRad = MIN_RAD;
		shootAng = PIovr2;
		shootVel = MIN_VEL;
		bhGrav = particles->bhV[0].surfaceAccel;
		
		resX = particles->resX;
		resY = particles->resY;
		shootLine[0] = sf::Vertex(sf::Vector2f(mouseX, mouseY));
		shootLine[1] = sf::Vertex(sf::Vector2f(mouseX + VEL_LINE_SCALE*shootVel*cos(shootAng),
																					 mouseY + VEL_LINE_SCALE*shootVel*sin(shootAng)));
		mouseCircle.setRadius(mouseRad);
		mouseCircle.setOutlineColor(sf::Color::White);
		mouseCircle.setOutlineThickness(1);
		
		windowFocused = true; // Probably true, but no way to know
		mousePressed = false;
		mouseHeld = false;
		mouseReleased = false;
		modeChanged = false;
		
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
			
			switch(mouseMode) {
				case 1: // Erase
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
						particles->deactivateCloud(mouseX, mouseY, mouseRad);
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
							particles->createCloud(mouseX, mouseY, mouseRad, 0, 0,
																			particles->defaultBallDia, particles->defaultBallSprRate,
																			particles->defaultBallebEff, particles->defaultBallAttrRate,
																			particles->defaultBallAttrRad, false, false);
						}
					}
					else if (paintForce) {
						if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseX <= *resX) { 
							particles->createCloud(mouseX, mouseY, mouseRad, 0, 0,
																			particles->defaultBallDia, particles->defaultBallSprRate,
																			particles->defaultBallebEff, particles->defaultBallAttrRate,
																			particles->defaultBallAttrRad, false, true);
						}
					}
					else if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseX <= *resX) {
						particles->createCloud(mouseX, mouseY, mouseRad, 0, 0,
																		particles->defaultBallDia, particles->defaultBallSprRate,
																		particles->defaultBallebEff, particles->defaultBallAttrRate,
																		particles->defaultBallAttrRad, false, false);
					}
					break;
				case 4: // Shoot
					if (mouseReleased && mouseX <= *resX) {
						particles->deactivateCloud(mouseX, mouseY, mouseRad);
						particles->createCloud(mouseX, mouseY, mouseRad, shootVel, shootAng,
																		particles->defaultBallDia, particles->defaultBallSprRate,
																		particles->defaultBallebEff, particles->defaultBallAttrRate,
																		particles->defaultBallAttrRad, false, false);
					}
					break;
				case 5: // Place Blackhole
					if (mouseReleased && mouseX <= *resX) {
						particles->createBH(mouseX, mouseY, bhGrav, bhRad*2.f, bhIinteract);
					}
					break;
				case 6: // Control Blackhole
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseX <= *resX) {
						if (mousePressed && !bhPermanent) {
							particles->bhV[0].x = mouseX;
							particles->bhV[0].y = mouseY;
							particles->bhV[0].active = true;
						}
						particles->bhV[0].xMove = mouseX;
						particles->bhV[0].yMove = mouseY;
					}
					else if (mouseReleased) {
						if (!bhPermanent) particles->bhV[0].active = false;
						else {
							particles->bhV[0].xMove = *resX/2.f;
							particles->bhV[0].yMove = *resY/2.f;
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
						shootAng -= event.mouseWheel.delta*ANG_TICK;
						shootAng = (shootAng < 0.f)?(PI2):((shootAng > PI2)?0.f:shootAng);
					}
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || 
							sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
						shootVel += event.mouseWheel.delta*VEL_TICK;
						shootVel = (shootVel < MIN_VEL)?(MIN_VEL):((shootVel > MAX_VEL)?MAX_VEL:shootVel);
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
						particles->bhV[0].setSize(particles->bhV[0].radius*2.f);
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
					mouseCircle.setRadius(mouseRad);
					mouseCircle.setPosition(mouseX-mouseRad, mouseY-mouseRad);
					mouseCircle.setFillColor(sf::Color::Transparent);
					shootLine[0] = sf::Vertex(sf::Vector2f(mouseX, mouseY));
					shootLine[1] = sf::Vertex(sf::Vector2f(mouseX + VEL_LINE_SCALE*shootVel*cos(shootAng),
																								 mouseY + VEL_LINE_SCALE*shootVel*sin(shootAng)));
					mainWindow->draw(mouseCircle);
					mainWindow->draw(shootLine, 2, sf::Lines);
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