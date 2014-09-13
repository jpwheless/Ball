// BALLS!
// Jon Wheless

// -------- Environmental variables --------

// Mostly for the ball objects.  All black hole variables are stored in that class

static final int numBalls = 600;
static final int maxInitialSpeed = 1; // Pixels per second
static final int ballDia = 10;
static final float ballRad = ballDia/2.f;

static final int height = 800;
static final int width = 1200;

BallModule[] ball;
BlackHole bh;

static final float mouseFilt = 0.05;
static final int flashDia = 300; // Must be more than bh.dia+2

boolean enableLinGrav = true;
static final float yGravity = 000.0;
static final float xGravity = 0.0;

boolean enableCollision = true;
static final float ballSpringRate = 50000.0; // pixels/sec^2 per pixel 
static final float ballRebEff = 0.7;

boolean enableSticky = true;
static final float ballSurface = 10000.0; // 10000 50000
static final float ballAttractRate = ballSurface*sq(ballRad); // pixels/sec^2 per pixel 
static final float ballAttractRad = 500.f; // 20 10

static final boolean debugOn = false;

// -------- Global variables -------- 

int lastDraw; // Draw timer
int lastDebug; // Debug readout timer

int ballCount2;
int ballCount1;

float accel, term; // Temporary radial gravity variable

float rad; // Temporary radial gravity variable

void setup() {
	size(width, height);
	frameRate(10000); // Arbitrarily high number so draw() loops as fast as it can
	
	ball = new BallModule[numBalls];
							
	bh = new BlackHole(width/2.0,  // Center X
											height/2.0, // Center Y
											-400000.0,	// Surface Accel
											20,			// Diameter
											1,			 // 1 = no collision, 2 = destruction,	3 = collision
											2);			// 1 = stationary,	2 = mouse control, 3 = permanent and mouse
								 
	// Initialize all ball objects
	for (int i = 0; i < numBalls; i++) {
		float xPos = 0, yPos = 0;
		int tryCount = 0;
		boolean collision = true;
		
		// Make sure ball doesn't collide with another upon start
		while (collision && tryCount <= 50) {
			xPos = random(ballRad, width-ballRad);
			yPos = random(ballRad, height-ballRad);
			collision = false;
			for (int j = 0; j < i && !collision; j++) {
				rad = sqrt(sq(xPos - ball[j].x) + sq(yPos - ball[j].y));
				if (rad < 2.f*ballDia) {
					collision = true;
				}
			}
			tryCount++;
		}
				
		ball[i] = new BallModule(xPos, yPos, random(maxInitialSpeed), random(0,TWO_PI));
	}
	
	lastDraw = millis() - 20;
	
	if (debugOn) lastDebug = millis() - 1000;
	
	frameRate = 1000; // Jump start frame rate average to remove physics errors upon startup
	
}


// Default looping function
// Computes ball positions as fast as it can, but updates screen at 60Hz
void draw() {
	
	bh.update();
	collisionUpdate();
	
	// Ball position update using velocities modified by above functions
	for (int i = 0; i < numBalls; i++) {
		ball[i].update();
	}
	
	// Draw at 60Hz, regardless of draw loop rate
	if (millis() - lastDraw > 17) {
		
		updateScreen();
		
		// reset draw timer
		lastDraw = millis();
		
	}
	
	if (debugOn && millis() - lastDebug > 1000) {
		
		for (int i = 0; i < numBalls; i++) {
			print("Particle ", i, ": Vel = ", sqrt(sq(ball[i].xVel) + sq(ball[i].yVel)));
			println(", x = ", ball[i].x, ", y = ", ball[i].y);
		}
		println();
		
		lastDebug = millis();
	}
	
	//noLoop();
	
}

// Handles particle collision, wall collision, and linear gravity
void collisionUpdate() {

	// Check for particle collisions
	if (enableCollision) {
		for (int i = 0; i < numBalls - 1; i++) {
			if (ball[i].enabled) {

				for (int j = i+1; j < numBalls; j++) {
					// Distance between the two points
					if (ball[j].enabled) {

						rad = sqrt(sq(ball[i].x - ball[j].x) + sq(ball[i].y - ball[j].y));
						if (rad < ballDia) { // Particles are colliding
							term = ballSpringRate*(ballDia-rad)/(frameRate);
							
							if (rad == 0) rad = 0.01; // Remove divide by zero errors
							
							accel = ((ball[i].x - ball[j].x)/rad)*term*
								(((ball[i].x < ball[j].x && ball[i].xVel < ball[j].xVel) ||
								(ball[i].x > ball[j].x && ball[i].xVel > ball[j].xVel)) ? ballRebEff : 1.0);
							ball[i].xVel += accel;
							ball[j].xVel -= accel;
							accel = ((ball[i].y - ball[j].y)/rad)*term*
								(((ball[i].y < ball[j].y && ball[i].yVel < ball[j].yVel) ||
								(ball[i].y > ball[j].y && ball[i].yVel > ball[j].yVel)) ? ballRebEff : 1.0);
							ball[i].yVel += accel;
							ball[j].yVel -= accel;
						}
						else if (enableSticky && rad < ballDia + ballAttractRad) {
							if (rad == 0) rad = 0.01;
							
							term = ballAttractRate/(sq(rad)*(frameRate));
							
							accel = ((ball[i].x - ball[j].x)/rad)*term;
							ball[i].xVel -= accel;
							ball[j].xVel += accel;
							accel = ((ball[i].y - ball[j].y)/rad)*term;
							ball[i].yVel -= accel;
							ball[j].yVel += accel;
						}
					}
				}
			}
		}
	}
	
	// bounce positions off screen limits
	// And do linear gravity
	ballCount1 = 0;
	ballCount2 = 0;
	for (int i = 0; i < numBalls; i++) {
		
		if (ball[i].enabled) {
			if (ball[i].x > width - ballRad) {
				// Ball linear spring rate w/ wall rebound efficiency
				ball[i].xVel += ((width-ballRad) - ball[i].x)*ballSpringRate*
					((ball[i].xVel < 0) ? ballRebEff : 1.0)/frameRate;
			}
			else if (ball[i].x < ballRad) {			
				ball[i].xVel += (ballRad - ball[i].x)*ballSpringRate*
					((ball[i].xVel > 0) ? ballRebEff : 1.0)/frameRate;
			}
			else {
				if (enableLinGrav) ball[i].xVel += xGravity/frameRate;
			}
			
			if (ball[i].y > height - ballRad) {
				ball[i].yVel += ((height-ballRad) - ball[i].y)*ballSpringRate*
					((ball[i].yVel < 0) ? ballRebEff : 1.0)/frameRate;
			}
			else if (ball[i].y < ballRad) {
				ball[i].yVel += (ballRad - ball[i].y)*ballSpringRate*
					((ball[i].yVel > 0) ? ballRebEff : 1.0)/frameRate;
			}
			else {
				if (enableLinGrav) ball[i].yVel += yGravity/frameRate;
			}
			
			ballCount1++;
			if (ball[i].x > 0 && ball[i].x < width && ball[i].y > 0 && ball[i].y < height) {
				// Count active balls
				ballCount2++;
			}
			
		}
	}
	
	
}

// Handles 60Hz refresh rate stuff
void updateScreen() {

	// Clear screen
	background(0);

	// Draw objects
	noStroke();
	for (int i = 0; i < numBalls; i++) {
		ball[i].draw();
	}
	
	bh.draw();
	
	// Frame rate readout
	fill(255, 255, 255);
	text(Float.toString(frameRate), 5, 12);
	 text(Integer.toString(ballCount1) + "," + Integer.toString(ballCount2), 5, 25);

}

// Individual ball objects
// Calculations performed by outside functions
class BallModule {
	public float x, y;
	public float xVel, yVel;
	public int rgb[] = {0,0,0};
	public boolean enabled;
	
	// Contructor
	BallModule(float xTemp, float yTemp, float speedTemp, float dirTemp) {
		
		// Position
		x = xTemp;
		y = yTemp;
		
		// Velocity vector
		xVel = speedTemp*cos(dirTemp);
		yVel = speedTemp*sin(dirTemp);
		
		// Set fill color
		rgb[0] = (int)random(255);
		rgb[1] = (int)random(255);
		rgb[2] = (int)random(255);
		
		enabled = true;
	}
	
	// Update position and velocity of ball
	void update() {
		if (enabled) {
			x += xVel/frameRate;
			y += yVel/frameRate;
			
			
			if (x < -ballRad || x > width+ballRad) {
				x = constrain(x, ballRad, width-ballRad);
				xVel = 0;
			}
			if (y < -ballRad || y > height+ballRad) {
				y = constrain(y, ballRad, height-ballRad);
				yVel = 0;
			}
			
			/*
			if (Double.isNaN(x)) {
				x = width/2.f;
				xVel = 0;
			}
			if (Double.isNaN(y)) {
				y = height/2.f;
				yVel = 0;
			}
			*/

			
		}
	}
	
	// Draw ball
	void draw() {
		if (enabled) {
			fill(rgb[0],rgb[1],rgb[2]);
			ellipse(x, y, ballDia, ballDia);
		}
	}
}

// Black hole object
class BlackHole {
	private float x, y, xPerm, yPerm, surfaceAccel, centerAccel, rad, term;
	private int dia, interact, user;
	private boolean flash;
	
	// Constructor
	// Center X
	// Center Y
	// Surface Accel
	// Diameter
	// 1 = no collision, 2 = destruction,	3 = collision
	// 1 = stationary,	2 = mouse control, 3 = permanent and mouse
	BlackHole(float xTemp, float yTemp, float surfaceAccelTemp, int diaTemp, int interactTemp, int userTemp) {
	
		xPerm = xTemp;
		x = xPerm;
		yPerm = yTemp;
		y = yPerm;
		dia = diaTemp;
		surfaceAccel = surfaceAccelTemp;
		centerAccel = surfaceAccel*sq(dia/2.0);
		interact = interactTemp;
		user = userTemp;
		
	}
	
	
	// Handles radial gravity from black hole
	void update() {
		
		// Update position of center gravity if mouse control enabled
		if (user > 1) {
			if (mousePressed) {
				x = mouseFilt*mouseX + (1.0-mouseFilt)*x;
				y = mouseFilt*mouseY + (1.0-mouseFilt)*y;
			}
			else if (user == 3) {
				x = mouseFilt*xPerm + (1.0-mouseFilt)*x;
				y = mouseFilt*yPerm + (1.0-mouseFilt)*y;
			}
		}
		
		// Handle ball collision with black hole particle
		// Check for particle collisions
		if (interact == 3) {
			for (int i = 0; i < numBalls - 1; i++) {
				if (ball[i].enabled) {

					rad = sqrt(sq(ball[i].x - x) + sq(ball[i].y - y));
					if (rad < (ballDia+dia)/2.0) { // Particles are colliding
						term = ballSpringRate*((ballDia+dia)/2.0 - rad)/(frameRate);
						
						ball[i].xVel += ((ball[i].x - x)/rad)*term;
						ball[i].yVel += ((ball[i].y - y)/rad)*term;
					}
				}
			}
		}
		
		if (user == 1 || user == 3 || (user == 2 && mousePressed)) {
			// Update velocity of each particle
			for (int i = 0; i < numBalls; i++) {
				if (ball[i].enabled) {
					rad = sqrt(sq(x - ball[i].x) + sq(y - ball[i].y));
					
					// Particle is above surface of black hole
					if (rad > (dia/2.0)) term = (centerAccel/sq(rad))/frameRate;
					
					// Particle center is below surface
					else {
						if (interact == 2) {
							ball[i].enabled = false;
							flash = true;
						}
						else term = surfaceAccel/frameRate;
					}
					
					ball[i].xVel += ((x - ball[i].x)/rad)*term;
					ball[i].yVel += ((y - ball[i].y)/rad)*term;
				}
			}
		}
	}
	
	void draw() {
		if (user == 1 || user == 3 || (user == 2 && mousePressed)) {
				
			// Draw a radial gradient
			if (flash) {
				noStroke();
				term = 2.0*255.0/(flashDia - dia - 2);
				int j = 0;
				for(int i = flashDia; i >= dia + 2; i -= 2) {
					j++;
					fill(j*term);
					ellipse(x, y, i, i);
				}
				flash = false;
			}
			
			fill(0,0,0);
			stroke(255);
			ellipse(x, y, dia, dia);	 
		}
	}
}
