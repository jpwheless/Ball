#define PI 3.14159265359
#define PI2 6.28318530718
#define PIovr2 1.57079632679
#define RAD_TICK 5
#define MAX_RAD 100
#define MIN_RAD particles->defaultBallDia + 2
#define VEL_TICK 200
#define MAX_VEL 8000
#define MIN_VEL 1000
#define ANG_TICK 0.0872664626
#define VEL_LINE_SCALE 0.05f

namespace z {

class Input {
public:
	z::Particles *particles;
	sf::RenderWindow *mainWindow;
	
	int *resX;
	
	int mouseX;
	int mouseY;
	int mouseMode;
	int mouseRad;
	double shootAng;
	double shootVel;
	
	bool windowFocused;
	bool mousePressed;
	bool mouseHeld;
	bool mouseReleased;
	
	sf::Vertex shootLine[2];
	
	sf::CircleShape mouseCircle;
	
	Input(z::Particles *p, sf::RenderWindow *mW) {
		particles = p;
		mainWindow = mW;
		mouseMode = 1;
		mouseRad = MIN_RAD;
		shootAng = PIovr2;
		shootVel = MIN_VEL;
		
		resX = particles->resX;
		shootLine[0] = sf::Vertex(sf::Vector2f(mouseX, mouseY));
		shootLine[1] = sf::Vertex(sf::Vector2f(mouseX + VEL_LINE_SCALE*shootVel*cos(shootAng),
																					 mouseY + VEL_LINE_SCALE*shootVel*sin(shootAng)));
		mouseCircle.setRadius(mouseRad);
		mouseCircle.setOutlineColor(sf::Color::White);
		mouseCircle.setOutlineThickness(1);
		mouseCircle.setFillColor(sf::Color::Transparent);
		
		windowFocused = true; // Probably true, but no way to know
		mousePressed = false;
		mouseHeld = false;
		mouseReleased = false;
	}
	
	void update() {
		if (windowFocused) {
			if (mousePressed) mouseHeld = true;
			else if (mouseReleased) mouseHeld = false;
			sf::Vector2i mousePos = sf::Mouse::getPosition(*mainWindow);
			mouseX = mousePos.x;
			mouseY = mousePos.y;
			if (mouseX <= *resX) {
				switch(mouseMode) {
					case 1:
						if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
							particles->deactivateCloud(mouseX, mouseY, mouseRad);
						}
						break;
					case 2:
						if (mousePressed) {
							particles->immobilizeCloud(mouseX, mouseY, mouseRad);
						}
						else if (mouseHeld) {
							particles->moveCloud(mouseX, mouseY);
						}
						else if (mouseReleased) {
							particles->mobilizeCloud();
						}
						break;
					case 3:
						if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
							particles->createCloud(mouseX, mouseY, mouseRad, 0, 0,
																			particles->defaultBallDia, particles->defaultBallSprRate,
																			particles->defaultBallebEff, particles->defaultBallAttrRate,
																			particles->defaultBallAttrRad, false);
						}
						break;
					case 4:
						if (mouseReleased) {
							particles->createCloud(mouseX, mouseY, mouseRad, shootVel, shootAng,
																			particles->defaultBallDia, particles->defaultBallSprRate,
																			particles->defaultBallebEff, particles->defaultBallAttrRate,
																			particles->defaultBallAttrRad, false);
						}
						break;
					default:
						break;
				}
			}
		}
		mousePressed = false;
		mouseReleased = false;
	}
		
	// Handle mouse wheel events
	void inputWheel(sf::Event &event) {
		if (windowFocused) {
			switch(mouseMode) {
				default:
				case 1:
				case 2:
					if (mouseHeld) break;
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
					mainWindow->draw(mouseCircle);
					break;
				case 4:
					mouseCircle.setRadius(mouseRad);
					mouseCircle.setPosition(mouseX-mouseRad, mouseY-mouseRad);
					shootLine[0] = sf::Vertex(sf::Vector2f(mouseX, mouseY));
					shootLine[1] = sf::Vertex(sf::Vector2f(mouseX + VEL_LINE_SCALE*shootVel*cos(shootAng),
																								 mouseY + VEL_LINE_SCALE*shootVel*sin(shootAng)));
					mainWindow->draw(shootLine, 2, sf::Lines);
					mainWindow->draw(mouseCircle);
					break;
				default:
					break;
			}
		}
	}
	
};

}