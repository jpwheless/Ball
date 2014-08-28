// BALLS!
// Jon Wheless

// -------- Environmental variables --------

// Mostly for the ball objects.  All black hole variables are stored in that class

static final int height = 800;
static final int width = 1005;

static final int numBalls = 1000;
static final int maxInitialSpeed = 0; // Pixels per second
static final int ballSize = 10;
BallModule[] ball;
BlackHole bh;

static final int border = (int)(ballSize/2.0);

static final float mouseFilt = 0.05;
static final int flashDia = 300; // Must be more than bh.dia+2

boolean enableLinGrav = true;
static final float yGravity = 500.0;
static final float xGravity = 0.0;

static final float wallSpringRate = 10000.0; // pixels/sec^2 per pixel
static final float wallRebEff = 0.01; // Wall rebound efficiency

boolean enableCollision = true;
static final float ballSpringRate = 10000.0; // pixels/sec^2 per pixel 

static final boolean debugOn = false;


// -------- Global variables -------- 

int lastDraw; // Draw timer
int lastDebug; // Debug readout timer
float term; // Temporary radial gravity variable
float rad; // Temporary radial gravity variable

void setup() {
   size(width, height);
   frameRate(10000); // Arbitrarily high number so draw() loops as fast as it can
   
   ball = new BallModule[numBalls];
                     
   bh = new BlackHole(width/2.0,  // Center X
                      height/2.0, // Center Y
                      100000.0,   // Surface Accel
                      100,         // Diameter
                      1,          // 1 = no collision, 2 = destruction,   3 = collision
                      2);         // 1 = stationary,   2 = mouse control, 3 = permanent and mouse
                         
   // Initialize all ball objects
   for (int i = 0; i < numBalls; i++) {
      ball[i] = new BallModule(random(width), random(height), random(maxInitialSpeed), random(0,TWO_PI));
   }
   
   lastDraw = millis();
   
   if (debugOn) lastDebug = millis();
   
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
                  if (rad < ballSize) { // Particles are colliding
                     term = (ballSpringRate*(ballSize-rad)/(frameRate));
                     
                     ball[j].xVel += ((ball[j].x - ball[i].x)/rad)*term;
                     ball[i].xVel += ((ball[i].x - ball[j].x)/rad)*term;
                     ball[j].yVel += ((ball[j].y - ball[i].y)/rad)*term;
                     ball[i].yVel += ((ball[i].y - ball[j].y)/rad)*term;
                  }
               }
            }
         }
      }
   }
   
   // bounce positions off screen limits
   // And do linear gravity
   for (int i = 0; i < numBalls; i++) {
      if (ball[i].enabled) {
         if (ball[i].x > width - border) {
            // Ball linear spring rate w/ wall rebound efficiency
            ball[i].xVel += ((width-border) - ball[i].x)*wallSpringRate*
               ((ball[i].xVel < 0) ? wallRebEff : 1.0)/frameRate;
         }
         else if (ball[i].x < border) {         
            ball[i].xVel += (border - ball[i].x)*wallSpringRate*
               ((ball[i].xVel > 0) ? wallRebEff : 1.0)/frameRate;
         }
         else {
            if (enableLinGrav) ball[i].xVel += xGravity/frameRate;
         }
         
         if (ball[i].y > height - border) {
            // Ball linear spring rate w/ wall rebound efficiency
            ball[i].yVel += ((height-border) - ball[i].y)*wallSpringRate*
               ((ball[i].yVel < 0) ? wallRebEff : 1.0)/frameRate;
         }
         else if (ball[i].y < border) {
            ball[i].yVel += (border - ball[i].y)*wallSpringRate*
               ((ball[i].yVel > 0) ? wallRebEff : 1.0)/frameRate;
         }
         else {
            if (enableLinGrav) ball[i].yVel += yGravity/frameRate;
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
   text(Float.toString(frameRate), 5, 15);

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
         
         x = constrain(x, 0, width);
         y = constrain(y, 0, height);
         
      }
   }
   
   // Draw ball
   void draw() {
      if (enabled) {
         fill(rgb[0],rgb[1],rgb[2]);
         ellipse(x, y, ballSize, ballSize);
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
   // 1 = no collision, 2 = destruction,   3 = collision
   // 1 = stationary,   2 = mouse control, 3 = permanent and mouse
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
               if (rad < (ballSize+dia)/2.0) { // Particles are colliding
                  term = ballSpringRate*((ballSize+dia)/2.0 - rad)/(frameRate);
                  
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
