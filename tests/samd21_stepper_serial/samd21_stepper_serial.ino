// for SparkFun SAMD21 Mini Breakout board with Pololu DRV8825 stepper driver
#include <SPI.h>

bool DEBUG_MODE = true;

const int CW = 0;
const int CCW = 1;

const int STEP_PIN = 2;
const int DIR_PIN = 3;
const int SIZE_PIN_0 = 4;
const int SIZE_PIN_1 = 5;
const int SIZE_PIN_2 = 6;

const int STEP_FULL = 1;
const int STEP_DIVIDE_2 = 2;
const int STEP_DIVIDE_4 = 4;
const int STEP_DIVIDE_8 = 8;
const int STEP_DIVIDE_16 = 16;
const int STEP_DIVIDE_32 = 32;
const int DO_STEP_MICROSECONDS_DELAY = 50;
const int BETWEEN_STEP_MICROSECONDS_DELAY = 500;

const int STEPS_PER_REVOLUTION = 200;           // for 1.8 deg stepper motor
const int DEG_ERROR_RANGE = 0.25;              // encoder accuracy
const int DEFAULT_BAUD_RATE = 115200;          // default serial baud rate

int currentPositionDeg = 0;                     // start from zero
int currentPositionMicrosteps32 = 0;            // transformed from origin deg as it should be 
int feedbackPositionDeg = 0;                   // read from the encoder
int feedbackPositionMicrosteps32 = 0;          // transformed from encoder read
int motorTemp = 0;                             // motor sensor temp
int dir = CW;
int stepSize = STEP_FULL;

// one time setup ops for program
void setup() {
  
  // step
  pinMode(STEP_PIN, OUTPUT);
  
  // dir
  pinMode(DIR_PIN, OUTPUT);

  // step size
  pinMode(SIZE_PIN_0, OUTPUT);
  pinMode(SIZE_PIN_1, OUTPUT);
  pinMode(SIZE_PIN_2, OUTPUT);

  // default step size full
  setStepSize(STEP_FULL);

  // default baud rate
  SerialUSB.begin(DEFAULT_BAUD_RATE);
  while(!SerialUSB) ; // Wait for Serial monitor to open

  // start the SPI library:
  pinMode(10, OUTPUT); // SPI Pin
  SPI.begin();
}

// set origin o current position
void resetOrigin(){
  currentPositionDeg = 0;
  currentPositionMicrosteps32 = 0;
  feedbackPositionDeg = 0;
  feedbackPositionMicrosteps32 = 0;
}

void getStatus(){
  String stat = "";
  stat = stat + "{deg:";
  stat = stat + currentPositionDeg;
  stat = stat + ",microsteps32:";
  stat = stat + currentPositionMicrosteps32;
  stat = stat + ",debug:";
  stat = stat + DEBUG_MODE;
  stat = stat + "}";
  SerialUSB.println(stat);
}

void readSPIAngle(){

  unsigned int result = 0;
  byte inByte = 0; 

  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  digitalWrite(10, LOW); // CS low

  // read first byte
  SPI.transfer(0);
  inByte = SPI.transfer(0x00);
  SerialUSB.println(inByte);
  result = inByte;

  // read second byte
  result = result << 8;
  inByte = SPI.transfer(0x00);
  SerialUSB.println(inByte);
  result = result | inByte; // combine the byte you just got with the previous one:

  SerialUSB.println(result);

  digitalWrite(10, HIGH); // CS high
  SPI.endTransaction();
}

void noDebug(){
  DEBUG_MODE = false;
}

void debug(){
  DEBUG_MODE = true;
}

// change direction pin output value
void setDir(int d){

  if(d == CW){
    digitalWrite(DIR_PIN, LOW);
  }
  else
  {
    digitalWrite(DIR_PIN, HIGH);
  }

  dir = d;
}

// change step size output values on control pins
void setStepSize(int size){

  if(size == STEP_FULL){
    digitalWrite(SIZE_PIN_0, LOW);
    digitalWrite(SIZE_PIN_1, LOW);
    digitalWrite(SIZE_PIN_2, LOW);
  }

  if(size == STEP_DIVIDE_2){
    digitalWrite(SIZE_PIN_0, LOW);
    digitalWrite(SIZE_PIN_1, LOW);
    digitalWrite(SIZE_PIN_2, HIGH);
  }

  if(size == STEP_DIVIDE_4){
    digitalWrite(SIZE_PIN_0, LOW);
    digitalWrite(SIZE_PIN_1, HIGH);
    digitalWrite(SIZE_PIN_2, LOW);
  }
  
  if(size == STEP_DIVIDE_8){
    digitalWrite(SIZE_PIN_0, LOW);
    digitalWrite(SIZE_PIN_1, HIGH);
    digitalWrite(SIZE_PIN_2, HIGH);
  }

  if(size == STEP_DIVIDE_16){
    digitalWrite(SIZE_PIN_0, HIGH);
    digitalWrite(SIZE_PIN_1, LOW);
    digitalWrite(SIZE_PIN_2, LOW);
  }

  if(size == STEP_DIVIDE_32){
    digitalWrite(SIZE_PIN_0, HIGH);
    digitalWrite(SIZE_PIN_1, HIGH);
    digitalWrite(SIZE_PIN_2, HIGH);
  }

  stepSize = size;
}

// do step as it is defined by control pins outputs
void doStep(int pin, int microseconds){
    digitalWrite(pin, LOW);
    delayMicroseconds(microseconds);
    digitalWrite(pin, HIGH);
    delayMicroseconds(microseconds);
    digitalWrite(pin, LOW);
    delayMicroseconds(microseconds);
}

// do multiple steps CW
void stepRight(int stepPin, int steps, int size, int microseconds, int after_step_microseconds){

    if(dir != CCW){
      setDir(CCW);
    }
    
    setStepSize(size);
    
    for (int s = 0; s <= steps; s++) {
      doStep(stepPin, microseconds);
      delayMicroseconds(after_step_microseconds);
    }

    setStepSize(STEP_FULL);
}

// do multiple steps CCW
void stepLeft(int stepPin, int steps, int size, int microseconds, int after_step_microseconds){

    if(dir != CW){
      setDir(CW);
    }

    setStepSize(size);

    for (int s = 0; s <= steps; s++) {
      doStep(stepPin, microseconds);
      delayMicroseconds(after_step_microseconds);   
    }

    setStepSize(STEP_FULL);
}

// main program loop
void loop(){

  SerialUSB.println("{status:'ONLINE'}");

  // forever read serial command and execute it
  while(true){

    String command = ""; // Create a new string
    while (SerialUSB.available())
    {
      command += (char)SerialUSB.read();
    }

    // COMMAND SHOULD BE OF STRING TEMPLATE explained below
    // L360.53 = Left 360.53 degrees
    // L36000.76 = Left 360 * 100 degrees + 0.76 deg
    // R360 = Right 360 deg

    command.trim();

    int len = command.length();
    if(len > 0){

      // echo back the command to confirm receive
      if(DEBUG_MODE){
        String resp = "{";
        resp = resp + "command:'";
        resp = resp + command;
        resp = resp + "'}";
        SerialUSB.println(resp);
      }

      if(command.equalsIgnoreCase("STATUS")){
         getStatus();
      }
      
      if(command.equalsIgnoreCase("ORIGIN")){
         resetOrigin();
         getStatus();
      }

      if(command.equalsIgnoreCase("DEBUG")){
        debug();
        getStatus();
      }

      if(command.equalsIgnoreCase("NODEBUG")){
        noDebug();
        getStatus();
      }

      if(command.equalsIgnoreCase("ANGLE")){
        readSPIAngle();
        getStatus();
      }

      char c0 = command[0];

      // move CCW
      if(c0 == 'L' || c0 == 'l'){

          // no of degrees to move
          double deg = command.substring(1).toDouble();

          int full = (int)(deg * 1.0 / (360.00 / STEPS_PER_REVOLUTION));
          
          if(full > 0){
            stepLeft(STEP_PIN, full, STEP_FULL, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
          }

          double degLeft = deg - (full * (360.00 / STEPS_PER_REVOLUTION));
          
          if(degLeft >= 360.00 / STEPS_PER_REVOLUTION / 2.0){
              stepLeft(STEP_PIN, 1, STEP_DIVIDE_2, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
              degLeft = degLeft - 360.00 / STEPS_PER_REVOLUTION / 2.0;
          }
          
          if(degLeft >= 360.00 / STEPS_PER_REVOLUTION / 4.0){
              stepLeft(STEP_PIN, 1, STEP_DIVIDE_4, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
              degLeft = degLeft - 360.00 / STEPS_PER_REVOLUTION / 4.0;
          }
          
          if(degLeft >= 360.00 / STEPS_PER_REVOLUTION / 8.0){
              stepLeft(STEP_PIN, 1, STEP_DIVIDE_8, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
              degLeft = degLeft - 360.00 / STEPS_PER_REVOLUTION / 8.0;
          }
          
          if(degLeft >= 360.00 / STEPS_PER_REVOLUTION / 16.0){
              stepLeft(STEP_PIN, 1, STEP_DIVIDE_16, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
              degLeft = degLeft - 360.00 / STEPS_PER_REVOLUTION / 16.0;
          }
          
          if(degLeft >= 360.00 / STEPS_PER_REVOLUTION / 32.0){
              int s32 = (int)(degLeft * 32 / (360.00 / STEPS_PER_REVOLUTION));
              stepLeft(STEP_PIN, s32, STEP_DIVIDE_32, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
          }

          // update status vars
          currentPositionDeg -= (deg - degLeft);
          currentPositionMicrosteps32 = currentPositionDeg * 32; 
          
          // confirm move with new status response
          getStatus();
      }

      // move CW
      if(c0 == 'R' || c0 == 'r'){

          // no of degrees to move
          double deg = command.substring(1).toDouble();

          int full = (int)(deg * 1.0 / (360.00 / STEPS_PER_REVOLUTION));
          
          if(full > 0){
            stepRight(STEP_PIN, full, STEP_FULL, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
          }

          double degRight = deg - (full * (360.00 / STEPS_PER_REVOLUTION));
          
          if(degRight >= 360.00 / STEPS_PER_REVOLUTION / 2.0){
              stepRight(STEP_PIN, 1, STEP_DIVIDE_2, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
              degRight = degRight - 360.00 / STEPS_PER_REVOLUTION / 2.0;
          }
          
          if(degRight >= 360.00 / STEPS_PER_REVOLUTION / 4.0){
              stepRight(STEP_PIN, 1, STEP_DIVIDE_4, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
              degRight = degRight - 360.00 / STEPS_PER_REVOLUTION / 4.0;
          }
          
          if(degRight >= 360.00 / STEPS_PER_REVOLUTION / 8.0){
              stepRight(STEP_PIN, 1, STEP_DIVIDE_8, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
              degRight = degRight - 360.00 / STEPS_PER_REVOLUTION / 8.0;
          }
          
          if(degRight >= 360.00 / STEPS_PER_REVOLUTION / 16.0){
              stepRight(STEP_PIN, 1, STEP_DIVIDE_16, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
              degRight = degRight - 360.00 / STEPS_PER_REVOLUTION / 16.0;
          }
          
          if(degRight >= 360.00 / STEPS_PER_REVOLUTION / 32.0){
              int s32 = (int)(degRight * 32 / (360.00 / STEPS_PER_REVOLUTION));
              stepRight(STEP_PIN, s32, STEP_DIVIDE_32, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);
          }

          // update status vars
          currentPositionDeg += (deg - degRight);
          currentPositionMicrosteps32 = currentPositionDeg * 32; 
          
          // confirm move with new status response
          getStatus();
      }
    }
  }
}
