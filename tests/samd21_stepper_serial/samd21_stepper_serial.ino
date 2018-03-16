// for SparkFun SAMD21 Mini Breakout board with Pololu DRV8825 stepper driver
const bool DEBUG_MODE = true;

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

int originPositionDeg = 0;                     // start from zero
int originPositionMicrosteps32 = 0;            // transformed from origin deg as it should be 
int feedbackPositionDeg = 0;                   // read from the encoder
int feedbackPositionMicrosteps32 = 0;          // transformed from encoder read
int motorTemp = 0;                             // motor sensor temp

int totalStepCount = 0;
int dir = CW;

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
}

// set origin o current position
void resetOrigin(){
  originPositionDeg = 0;
  originPositionMicrosteps32 = 0;
  feedbackPositionDeg = 0;
  feedbackPositionMicrosteps32 = 0;
}

void getStatus(){
  String stat = "";
  stat = stat + "{deg:";
  stat = stat + originPositionDeg;
  stat = stat + ",microsteps32:";
  stat = stat + originPositionMicrosteps32;
  stat = stat + "}";
  SerialUSB.println(stat);
}

// change direction pin output value
void setDir(int dir){

  if(dir == CW){
    digitalWrite(DIR_PIN, LOW);
  }
  else
  {
    digitalWrite(DIR_PIN, HIGH);
  }
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

    setDir(CCW);
    setStepSize(size);
    
    for (int s = 0; s < steps; s++) {
      doStep(stepPin, microseconds);
      delayMicroseconds(after_step_microseconds);
    }
}

// do multiple steps CCW
void stepLeft(int stepPin, int steps, int size, int microseconds, int after_step_microseconds){

    setDir(CW);
    setStepSize(size);

    for (int s = 0; s < steps; s++) {
      doStep(stepPin, microseconds);
      delayMicroseconds(after_step_microseconds);   
    }
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

      // move CCW
      if(command[0] == 'L' || command[0] == 'l'){

          // no of degrees to move
          double deg = command.substring(1).toDouble();

          int full = (int)(deg / (360 / STEPS_PER_REVOLUTION));
          stepLeft(STEP_PIN, full, STEP_FULL, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);

          // update status vars
          originPositionDeg -= deg;
          originPositionMicrosteps32 = originPositionDeg * 32; 
          
          // confirm move with new status response
          getStatus();
      }

      // move CW
      if(command[0] == 'R' || command[0] == 'r'){

          // no of degrees to move
          double deg = command.substring(1).toDouble();

          int full = (int)(deg / (360 / STEPS_PER_REVOLUTION));
          stepRight(STEP_PIN, full, STEP_FULL, DO_STEP_MICROSECONDS_DELAY, BETWEEN_STEP_MICROSECONDS_DELAY);

          // update status vars
          originPositionDeg += deg;
          originPositionMicrosteps32 = originPositionDeg * 32; 
          
          // confirm move with new status response
          getStatus();
      }
    }
  }
}
