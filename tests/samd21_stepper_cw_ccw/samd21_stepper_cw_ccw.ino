// for SparkFun SAMD21 Mini Breakout board with Pololu DRV8825 stepper driver
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
const int STEP_MICROSECONDS_DELAY = 50;

int stepCount = 0;
int dir = CW;

void setup() {
  
  // step
  pinMode(STEP_PIN, OUTPUT);
  
  // dir
  pinMode(DIR_PIN, OUTPUT);

  // step size
  pinMode(SIZE_PIN_0, OUTPUT);
  pinMode(SIZE_PIN_1, OUTPUT);
  pinMode(SIZE_PIN_2, OUTPUT);

  //default step size full
  setStepSize(STEP_FULL);
}

void setDir(int dir){

  if(dir == CW){
    digitalWrite(DIR_PIN, LOW);
  }
  else
  {
    digitalWrite(DIR_PIN, HIGH);
  }
}


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

void doStep(int pin, int microseconds){
    digitalWrite(pin, LOW);
    delayMicroseconds(microseconds);
    digitalWrite(pin, HIGH);
    delayMicroseconds(microseconds);
    digitalWrite(pin, LOW);
    delayMicroseconds(microseconds);
}

void stepLeft(int stepPin, int steps, int size, int microseconds){

    setDir(CCW);
    setStepSize(size);
    
    for (int s = 0; s < steps; s++) {
      doStep(stepPin, microseconds);
    }
}

void stepRight(int stepPin, int steps, int size, int microseconds){

    setDir(CW);
    setStepSize(size);

    for (int s = 0; s < steps; s++) {
      doStep(stepPin, microseconds);    
    }
}


void loop(){

    if(stepCount > 200){
      
      if(dir == CW){
        dir = CCW;
      }
      else
      {
        dir = CW;
      }
      
      stepCount = 0;
    }

    if(dir == 0){
      stepLeft(STEP_PIN, 1, STEP_DIVIDE_32, STEP_MICROSECONDS_DELAY);
    }
    else
    {
      stepRight(STEP_PIN, 1, STEP_DIVIDE_32, STEP_MICROSECONDS_DELAY);
    }
    
    stepCount++;
    delay(1);
}
