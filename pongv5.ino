/**
 * Pong v5
 * 
 * See http://blog.ampli.fi/dyi-pong-console-based-on-arduino-and-an-rgb-led-matrix
 * 
 */

#include <RGBmatrixPanel.h>
#define CLK  8   // USE THIS ON ARDUINO UNO, ADAFRUIT METRO M0, etc.
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);


// Number font definitions
boolean numbers[10][7][5] {
  {
    {false,true,true,true,false},
    {true,false,false,false,true},
    {true,false,false,true,true},
    {true,false,true,false,true},
    {true,true,false,false,true},
    {true,false,false,false,true},
    {false,true,true,true,false}
  },
  {
    {false,false,true,false,false},
    {false,true,true,false,false},
    {false,false,true,false,false},
    {false,false,true,false,false},
    {false,false,true,false,false},
    {false,false,true,false,false},
    {false,true,true,true,false}
  },
  {
    {false,true,true,true,false},
    {true,false,false,false,true},
    {false,false,false,false,true},
    {false,false,false,true,false},
    {false,false,true,false,false},
    {false,true,false,false,false},
    {true,true,true,true,true}
  },
  {
    {true,true,true,true,true},
    {false,false,false,true,false},
    {false,false,true,false,false},
    {false,false,false,true,false},
    {false,false,false,false,true},
    {true,false,false,false,true},
    {false,true,true,true,false}
  },
  {
    {false,false,false,true,false},
    {false,false,true,true,false},
    {false,true,false,true,false},
    {true,false,false,true,false},
    {true,true,true,true,true},
    {false,false,false,true,false},
    {false,false,false,true,false}
  },
  {
    {true,true,true,true,true},
    {true,false,false,false,false},
    {true,true,true,true,false},
    {false,false,false,false,true},
    {false,false,false,false,true},
    {true,false,false,false,true},
    {false,true,true,true,false}
  },
  {
    {false,false,true,true,false},
    {false,true,false,false,false},
    {true,false,false,false,false},
    {true,true,true,true,true},
    {true,false,false,false,true},
    {true,false,false,false,true},
    {false,true,true,true,false}
  },
  {
    {true,true,true,true,true},
    {true,false,false,false,true},
    {false,false,false,true,false},
    {false,false,true,false,false},
    {false,true,false,false,false},
    {false,true,false,false,false},
    {false,true,false,false,false}
  },
  {
    {false,true,true,true,false},
    {true,false,false,false,true},
    {true,false,false,false,true},
    {false,true,true,true,false},
    {true,false,false,false,true},
    {true,false,false,false,true},
    {false,true,true,true,false}
  },
  {
    {false,true,true,true,false},
    {true,false,false,false,true},
    {true,false,false,false,true},
    {false,true,true,true,true},
    {false,false,false,false,true},
    {false,false,false,true,false},
    {false,true,true,false,false}
  }
};


// Initial PixelColor
unsigned int ipc = matrix.Color333(3, 3, 3);
unsigned int pc = ipc;
// Initial AntialiasPixelColor
unsigned int iapc = matrix.Color333(1, 1, 1);
unsigned int apc = iapc;

// Standby mode color changes
unsigned long lastColorChangeTime;
int pcr = 0, pcg = 0, pcb = 0, apcr, apcg, apcb;

// Initial location of ball
byte ballyi = 15;
byte ballxi = 7;

// x coordinate of the paddle in the game
byte x1;
byte x2;

const byte GAME_STANDBY = 0;
const byte GAME_START = 1;
const byte GAME_PLAY = 2;
const byte GAME_OVER = 3;

// Variables that are reset at the beginning of each game:
// Current location
float ballx;
float bally;
// Initial direction of travel as 1/16ths of the arc of a circle.
// Direction 0 is to the right and direction increases CCW.
int ball_direction;
byte game_state;
byte score1;
byte score2;
unsigned long score_time;
unsigned long game_over_time;
unsigned long last_fireworks_time;
// Velocity constant, defines ball speed.
float vc;


// Stores value of the potentiometer
int paddle1;
int paddle2;
// Stores the values of the pots at the end of game to enable starting the
// next game by turning a paddle.
int standbyPaddle1;
int standbyPaddle2;

float rad, dx, dy, angle;

boolean debugging = false;

unsigned long lastScreenUpdate;
byte refreshDelay = 10;

// Delay after scoring in ms
int score_delay = 2000;

/**
 * 
 */
void setup() {
  // Debugger
  // Delay before reserving the serial port
  if (debugging) {
    delay(1000);
    Serial.begin(57600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    debug("Debugger active");
  }

  matrix.begin();

  // Save the initial positions of the paddles before loop() runs.
  paddle1 = analogRead(A4);
  paddle2 = analogRead(A5);
  standbyPaddle1 = paddle1;
  standbyPaddle2 = paddle2;
  
  setStandby();
}

/**
 *  
 */
void loop() {
  paddle1 = analogRead(A4);
  paddle2 = analogRead(A5);

  x1 = map(paddle1, 0, 1023, 1, 14);
  x2 = map(paddle2, 0, 1023, 14, 1);

  if (millis() - lastScreenUpdate > refreshDelay) {
    updateScreen();
    lastScreenUpdate = millis();
  }
}


/**
 * Reset values at the beginning of the game.
 */
void setPlay() {
  game_state = GAME_PLAY;
  ball_direction = 2;
  score1 = 0;
  score2 = 0;
  ballx = ballxi;
  bally = ballyi;
  score_time = millis();
  last_fireworks_time = 0;
  vc = (float) refreshDelay / 80;
  lastScreenUpdate = 0;  
  pc = ipc;
  apc = iapc;
}


/**
 * Reset values when going to standby.
 */
void setStandby() {
  game_state = GAME_STANDBY;
  standbyPaddle1 = paddle1;
  standbyPaddle2 = paddle2;
  ballx = ballxi;
  bally = ballyi;
  ball_direction = 2;
  vc = (float) refreshDelay / 160;
}

void debug(String s) {
  if (debugging) {
    Serial.println(s);    
  }
}

void score(byte player) {
  ballx = ballxi;
  bally = ballyi;
  score_time = millis();

  float vci = (score1 + score2) * 0.015;
  vc = ((float) refreshDelay / 80) + vci;
  
  if (player == 1) {
    score1++;
    ball_direction = 10;

    if (score1 == 10) {
      win(1);
    }
  }
  else {
    score2++;
    ball_direction = 2;

    if (score2 == 10) {
      win(2);
    }
  }
}


void win(int player) {
  game_state = GAME_OVER;
  game_over_time = millis();
}


/**
 * Draws a pixel in the given color
 */
void pixelc(float y, float x, unsigned int color) {
  matrix.drawPixel(round(y), round(x), color);
}

/**
 * Draws a pixel in the default color
 */
void pixel(float y, float x) {
   matrix.drawPixel(round(y), round(x), pc);
}

/** 
 * Draw an antialiased pixel using the default color.
 */
void pixela(float y, float x) {
  byte yr = round(y);
  byte xr = round(x);

  // First draw the regular ol' pixel
  pixel(yr, xr);

  // Calculate how much off an exact integer we are
  float xd = x - xr;
  float yd = y - yr;

  // Draw antialiasing pixels if the offset values are big enough.
  if (abs(xd) > 0.25) {
    if (xd > 0) {
      // if negative, alias bigger
      pixelc(yr, xr+1, apc);
    }
    else {
      // if positive, alias smaller
      pixelc(yr, xr-1, apc);
    }
  }
  if (abs(yd) > 0.25) {
    if (yd > 0) {
      // if negative, alias bigger
      pixelc(yr+1, xr, apc);
    }
    else {
      // if positive, alias smaller
      pixelc(yr-1, xr, apc);
    }
  }
}

/**
 *  The main function that determines ball position and direction.
 */
void updatePos() {
  rad = (float(ball_direction)/16) * 2 * PI;
  dy = sin(rad) * vc;
  dx = cos(rad) * vc;

  // Only update position if the change doesn't take the ball out of
  // bounds.
  if (ballx + dx >= 0 && ballx + dx <= 15) {
    if (bally + dy >= 1 && bally + dy <= 30) {
      // Update position
      ballx += dx;
      bally += dy;

      return;
    }
  }

  // Ball is getting out of bounds; bounce it.

  if (game_state == GAME_PLAY) {
    // Increase ball speed just a little bit after each bounce.
    vc += 0.003;
  }

  // First, change the dx and dy as needed.
  int direction_diff = 0;
  if (bally + dy > 30) {
    // Top bounce.
    if (game_state == GAME_STANDBY) {
      direction_diff = 1;
    }
    else {
      // Adjust the bounce direction based on where on the paddle the ball
      // hit.
      direction_diff = round(x2 - ballx);
    }

    // If the ball is directly below the paddle, i.e. their x positions
    // differ by no more than 1.
    if (abs(x2 - round(ballx)) <= 1 || game_state == GAME_STANDBY) {
      dy = 0 - dy;
    }
    // If the next move would make the ball and paddle intersect. This
    // is relevant when the ball hits the "corner" of the paddle.
    else if (round(ballx) + dx > x2 - 2 && round(ballx) + dx < x2 + 2) {
      dy = 0 - dy;
    }
    else {
      score(1);
      return;
    }
  }
  else if (bally + dy < 1) {
    // Bottom bounce.
    if (game_state == GAME_STANDBY) {
      direction_diff = -1;
    }
    else {
      // Adjust the bounce direction based on where on the paddle the ball
      // hit.
      direction_diff = round(ballx - x1); 
    }

    // If the ball is directly above the paddle, i.e. their x positions
    // differ by no more than 1.
    if (abs(x1 - round(ballx)) <= 1  || game_state == GAME_STANDBY) {
      dy = 0 - dy;
    }
    // If the next move would make the ball and paddle intersect. This
    // is relevant when the ball hits the "corner" of the paddle.
    else if (round(ballx) + dx > x1 - 2 && round(ballx) + dx < x1 + 2) {
      dy = 0 - dy;
    }
    else {
      score(2);
      return;
    }
  }

  if (ballx + dx < 0 || ballx + dx > 15) {
    // Right/left bounce. Reverse dx
    dx = 0 - dx;
  }

  // Then, find a direction that matches the new dx and dy
  angle = atan2(dy, dx);

  // Convert rads to 1/16ths of the arc
  ball_direction = round(angle / (PI * 2 / 16));

  // Adjust the direction based on where on the paddle the ball hit.
  ball_direction = ball_direction - direction_diff;

  // Make sure we don't go to directions 0 or 8, which would put
  // the ball going sideways and unplayable
  if (ball_direction == 0 || abs(ball_direction) == 8) {
    // Move the value of the direction diff towards 0.
    // -3  -  (-3 / abs(-3)) => -3  -  (-1) = -2
    // 3   -  (3  / abs(3))  => 3   -  (1)  = 2
    direction_diff = direction_diff - (direction_diff / abs(direction_diff));

    ball_direction = ball_direction + direction_diff;
  }

  if (ball_direction < 0) {
    ball_direction += 16;
  }
  if (ball_direction > 15) {
    ball_direction -= 16;
  }
}


/**
 * 
 */
void updateScreen() {
  if (game_state == GAME_STANDBY) {
    // Display a bouncing ball as a screen saver
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    standbyColor();
    drawBall();
    updatePos();
    
    // Start game if paddles are touched.
    if (abs(paddle1 - standbyPaddle1) > 10 || abs(paddle2 - standbyPaddle2) > 10) {
      game_state = GAME_START;
    }
  }
  else if (game_state == GAME_START) {
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    setPlay();
    drawPaddles();
    drawBall();
  }
  else if (game_state == GAME_PLAY ) {
    if (millis() - score_time < 2000) {
      // If game just started or someone just scored, wait a while before
      // starting moving the ball. While we wait, we display the current
      // score.

      drawNumber(score1, 2, 1, false);
      drawNumber(score2, 2, 10, false);

      drawNumber(score1, 23, 1, true);
      drawNumber(score2, 23, 10, true);

      // Score hyphens
      pixel(5, 7);
      pixel(5, 8);
      pixel(26, 7);
      pixel(26, 8);

      wipePaddles();
      drawPaddles(); 
    }
    else {
      // During a round we won't display the score.
      
      // Wipe screen
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      drawPaddles();
      drawBall();
      updatePos();
    }
  }
  else if (game_state == GAME_OVER) {
    if (score1 == 10) {
      drawNumber(1, 2, 3, false);
      drawNumber(0, 2, 8, false);
    }
    else if (score2 == 10) {
      drawNumber(1, 23, 8, true);
      drawNumber(0, 23, 3, true);
    }

    // Wait for a bit and then set game state to standby
    if (millis() - game_over_time < 4000) {
      return;
    }
    else {
      // Wipe screen
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      game_state = GAME_STANDBY;
      setStandby();  
    }  
  }
}

/**
 * @param int number The number to be drawn
 * @param int y The y coordinate of the bottom left corner of the number
 * @param int x The x coordinate of the bottom left corner of the number
 * @param boolean inverted Draw the number upside down?
 *
 */
void drawNumber(byte number, byte y, byte x, boolean inverted) {
  for (byte yp = 0; yp < 7; yp++) {
    for (byte xp = 0; xp < 5; xp++) {
      boolean value = numbers[number][yp][xp];

      if (value) {
        if (inverted) {
          pixel(yp + y, (4 - xp) + x);
        }
        else {
          pixel((6 - yp) + y, xp + x);
        }
      }
    }
  }
}


void drawPaddles() {
  pixel(0, x1-1);
  pixel(0, x1);
  pixel(0, x1+1);

  pixel(31, x2-1);
  pixel(31, x2);
  pixel(31, x2+1);
}

void drawBall() {
  pixela(bally, ballx);  
}

/**
 *  Wipe paddles only
 */
void wipePaddles() {
  matrix.drawLine(0, 0, 0, 15, matrix.Color333(0, 0, 0));
  matrix.drawLine(31, 0, 31, 15, matrix.Color333(0, 0, 0));
}

/**
 * Cycle pixel color during standby mode
 */
void standbyColor() {
  if (millis() - lastColorChangeTime > 500) {

    if (pcr == 0 && pcg == 0) {
      pcb += 1;
      if (pcb > 7) {
        pcb = 7;
        pcg += 1;
      }
    }
    else if (pcr == 0 && pcb == 7) {
      pcg += 1;
      if (pcg > 7) {
        pcg = 7;
        pcb -= 1;
      }
    }
    else if (pcr == 0 && pcg == 7) {
      pcb -= 1;
      if (pcb < 0) {
        pcb = 0;
        pcr += 1;
      }
    }
    else if (pcg == 7 && pcb == 0) {
      pcr += 1;
      if (pcr > 7) {
        pcr = 7;
        pcb += 1;
      }
    }
    else if (pcr == 7 && pcg == 7) {
      pcb += 1;
      if (pcb > 7) {
        pcb = 7;
        pcg -= 1;
      }
    }
    else if (pcr == 7 && pcb == 7) {
      pcg -= 1;
      if (pcg < 0) {
        pcg = 0;
        pcb -= 1;
      }
    }
    else if (pcr == 7 && pcg == 0) {
      pcb -= 1;
      if (pcb < 0) {
        pcb = 0;
        pcr -= 1;
      }
    }
    else if (pcb == 0 && pcg == 0) {
      pcr -= 1;
      if (pcr < 0) {
        pcr = 0;
        pcb += 1;
      }
    }
    else {
      debug("Oops");  
    }


    pc = matrix.Color333(pcr, pcg, pcb);

    apcr = pcr - 2;
    if (apcr < 0 ) { apcr = 0;}
    apcg = pcg - 2;
    if (apcg < 0 ) { apcg = 0;}
    apcb = pcb - 2;
    if (apcb < 0 ) { apcb = 0;}

    apc = matrix.Color333(apcr, apcg, apcb);
    
    lastColorChangeTime = millis();
  }  
}
