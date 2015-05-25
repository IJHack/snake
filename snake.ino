// Snake on 8x8Matrix 
// 2013-06-15 JorgVisch
// 
// IJduino port 
// 2015-05-25 AnneJan
//
#include "LedControl.h"

static const bool wrap = false;

/* using VCC, GND, DIN 20, CS 21, CLK 5 for the MAX7219 */
static const int DATA_PIN = 20;
static const int CLK_PIN  = 5;
static const int CS_PIN   = 21;

LedControl lc=LedControl(DATA_PIN, CLK_PIN, CS_PIN, 1);

// Button pin
const int buttonRightPin = 9;
const int buttonLeftPin  = 10;

static const int SOUND_PIN = 8; // piezo element for sound effects

// Game constants
// buttons
const int RIGHTBUTTON = 0;
const int LEFTBUTTON  = 1;
// direction
const int TOP    = 0;
const int RIGHT  = 1;
const int BOTTOM = 2;
const int LEFT   = 3;
// Snake
const int MAX_SNAKE_LENGTH = 64;

// Variables
int direction = TOP;                               // direction of movement
int snakeX[MAX_SNAKE_LENGTH];                      // X-coordinates of snake
int snakeY[MAX_SNAKE_LENGTH];                      // Y-coordinates of snake
int snakeLength = 1;                               // nr of parts of snake
boolean buttonRead = false;                        // is button already read in this loop
unsigned long prevTime = 0;                        // for gamedelay (ms)
unsigned long delayTime = 500;                     // Game step in ms

int fruitX, fruitY;
unsigned long fruitPrevTime = 0;
unsigned long fruitBlinkTime = 1000/250;
bool fruitLed = true;
bool playing = false;
bool recording_play=false;

void setup(){
  Serial.begin(9600);
  Serial.println("Snake is started");
  randomSeed(analogRead(0));
  // Init led matrix
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,15);
  /* and clear the display */
  lc.clearDisplay(0);  // init buttons
  int buttonpins[] = {buttonRightPin, buttonLeftPin};
  initButtons(buttonpins, 2);
  // init snake
  snakeX[0] = 4;
  snakeY[0] = 4;
  for(int i=1; i<MAX_SNAKE_LENGTH; i++){
    snakeX[i] = snakeY[i] = -1;
  }
  makeFruit();
}

void loop(){
  checkButtons();
  unsigned long currentTime = millis();
  if(currentTime - prevTime >= delayTime){    
    if (playing) {
        nextstep(); 
        buttonRead = false;
        prevTime = currentTime;
    } else {
      checkButtons();
      if (buttonRead) {
         buttonRead = false;
         playing = true;
      }
    }
  }
  draw();
}

void checkButtons(){
  if(!buttonRead){
    int currentDirection = direction;
    if(buttonClicked(LEFTBUTTON)){
      direction--;
      if(direction < 0){
        direction = LEFT;
      }
    }
    else if(buttonClicked(RIGHTBUTTON)){
      direction++;
      if(direction > 3){
        direction = TOP;
      }
    }
    buttonRead = (currentDirection != direction);
  }
}

void draw(){
  lc.clearDisplay(0);
  drawSnake();
  drawFruit();
  //matrix.writeDisplay();
}

void drawSnake(){
  for(int i=0; i<snakeLength; i++){
    lc.setLed(0, snakeX[i], snakeY[i], true);
  }
}

void drawFruit(){
  if(inPlayField(fruitX, fruitY)){
    unsigned long currenttime = millis();
    if(currenttime - fruitPrevTime >= fruitBlinkTime){
      fruitLed = fruitLed ? false : true;
      fruitPrevTime = currenttime;
    }
    lc.setLed(0, fruitX, fruitY, fruitLed);
  }
}

boolean inPlayField(int x, int y){
  return (x>=0) && (x<8) && (y>=0) && (y<8);
}

void nextstep(){
  for(int i=snakeLength-1; i>0; i--){
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1];
  }
  if (!moveIt()) {
    died();
  }
  if((snakeX[0] == fruitX) && (snakeY[0] == fruitY)){
    snakeLength++;
    if(snakeLength < MAX_SNAKE_LENGTH){  
      sound(1000, 100);    
      makeFruit();
    } 
    else {
      fruitX = fruitY = -1;
    }
  }
  if (snakeX[0] < 0) {
    if (wrap) {
      snakeX[0] = 7;
    } else {
      died();
    }
  }
  if (snakeY[0] < 0) {
    if (wrap) {
      snakeY[0] = 7;
    } else {
      died();
    }
  }
  if (snakeX[0] > 7) {
    if (wrap) {
      snakeX[0] = 0;
    } else {
      died();
    }
  }
  if (snakeY[0] > 7) {
    if (wrap) {
      snakeY[0] = 0;
    } else {
      died();
    }
  }
  
}

void makeFruit(){
  int x, y;
  x = random(0, 8);
  y = random(0, 8);
  while(isPartOfSnake(x, y)){
    x = random(0, 8);
    y = random(0, 8);
  }
  fruitX = x;
  fruitY = y;
}

boolean isPartOfSnake(int x, int y){
  for(int i=0; i<snakeLength-1; i++){
    if((x == snakeX[i]) && (y == snakeY[i])){
      return true;
    }
  }
  return false;
}

void sound(int freq, int duration)
{
  if (!recording_play)
    tone(SOUND_PIN, freq, duration);
}

bool moveIt() {
  switch(direction){
    case TOP:
      if (isPartOfSnake(snakeX[0], snakeY[0]-1)) {
        return false; 
      }
      snakeY[0] = snakeY[0]-1;
      break;
    case RIGHT:
      if (isPartOfSnake(snakeX[0]+1, snakeY[0])) {
        return false; 
      }
      snakeX[0] = snakeX[0]+1;
      break;
    case BOTTOM:
      if (isPartOfSnake(snakeX[0], snakeY[0]+1)) {
        return false; 
      }
      snakeY[0] = snakeY[0]+1;
      break;
    case LEFT:
      if (isPartOfSnake(snakeX[0]-1, snakeY[0])) {
        return false; 
      }
      snakeX[0]=snakeX[0]-1;
      break;
  }
  return true;
}

void died() {
  for (int i=0; i<10; i++) 
  {
    sound( 2000- (i*200), 200);
    delay(100);
  }
  snakeLength = 1;  
  snakeX[0] = 4;
  snakeY[0] = 4;
  for(int i=1; i<MAX_SNAKE_LENGTH; i++){
    snakeX[i] = snakeY[i] = -1;
  }
  playing = false;
}
