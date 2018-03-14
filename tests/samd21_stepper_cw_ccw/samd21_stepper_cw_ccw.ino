
const int CW = 0;
const int CCW = 1;

int stepPin = 2;
int dirPin = 3;

int stepCount = 0;
int dir = CW;

void setup() {
  
  // step
  pinMode(stepPin, OUTPUT);
  
  // dir
  pinMode(dirPin, OUTPUT);
}

void setDir(int dir){

  if(dir == CW){
    digitalWrite(dirPin, LOW);
  }
  else
  {
    digitalWrite(dirPin, HIGH);
  }
}

void doStep(int pin){
    digitalWrite(pin, LOW);
    delayMicroseconds(10);
    digitalWrite(pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin, LOW);
}

void stepLeft(int steps, int ms){

    setDir(CCW);
    
    for (int s = 0; s < steps; s++) {
      doStep(stepPin);
      delay(ms);
    }
}

void stepRight(int steps, int ms){

    setDir(CW);

    for (int s = 0; s < steps; s++) {
      doStep(stepPin);
      delay(ms);    
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
      stepLeft(1, 0);
    }
    else
    {
      stepRight(1, 0);
    }
    
    stepCount++;
    delay(1);
}
