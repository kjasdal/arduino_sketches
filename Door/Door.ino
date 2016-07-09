#include <Arduino.h>
#include <pins_arduino.h>
#include <Bounce2.h>


#define DEBUG

#define I_PIN 2
#define O_PIN 3

#define L_PIN 13

enum BeamState { OPEN, CLOSED };

enum State { IDLE, I_STEP_1, I_STEP_2, I_STEP_3, O_STEP_1, O_STEP_2, O_STEP_3, ERROR };

typedef struct {
  int state = IDLE;
  int iValid = false;
  int oValid = false;
} Context;

static Context context;

static Bounce iPinBounce;
static Bounce oPinBounce;

static int count = 0;

static int LEDCount = 10000;
static int LEDState = HIGH;

static void iBeamStateChangedToOpen(Context &context);
static void iBeamStateChangedToClosed(Context &context);
static void oBeamStateChangedToOpen(Context &context);
static void oBeamStateChangedToClosed(Context &context);

static String beamStateAsString(int state);
static String stateAsString(int state);

static int iValid(Context &context);
static int oValid(Context &context);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println();

  pinMode(13, OUTPUT);
//  digitalWrite(L_PIN, state);
  
  pinMode(I_PIN, INPUT_PULLUP);
  iPinBounce.attach(I_PIN);
  iPinBounce.interval(5);
  
  pinMode(O_PIN, INPUT_PULLUP);
  oPinBounce.attach(O_PIN);
  oPinBounce.interval(5);

  Serial.print("Count: ");
  Serial.println(count);

#ifdef DEBUG
  Serial.print("State: ");
  Serial.println(stateAsString(context.state));
#endif  
}

void loop() {
  int iBeamState;
  int oBeamState;
  
  if (iPinBounce.update() == 1) {
    iBeamState = iPinBounce.read();
#ifdef DEBUG
    Serial.print("iBeam: ");
    Serial.println(beamStateAsString(iBeamState));
#endif
    switch (iBeamState) {
      case OPEN:
        //iBeamStateChangedToOpen(state);
        iBeamStateChangedToOpen(context);
        break;
      case CLOSED:
        //iBeamStateChangedToClosed(state);
        iBeamStateChangedToClosed(context);
        break;
    }
  }
  if (oPinBounce.update() == 1) {
    oBeamState = oPinBounce.read();
#ifdef DEBUG
    Serial.print("oBeam: ");
    Serial.println(beamStateAsString(oBeamState));
#endif
    switch (oBeamState) {
      case OPEN:
        //oBeamStateChangedToOpen(state);
        oBeamStateChangedToOpen(context);
        break;
      case CLOSED:
        //oBeamStateChangedToClosed(state);
        oBeamStateChangedToClosed(context);
        break;
    }
  }

  if (iValid(context)) {
    count++;
    Serial.print("Count: ");
    Serial.println(count);
  }

  if (oValid(context)) {
    count--;
    Serial.print("Count: ");
    Serial.println(count);
  }

  LEDCount--;
  if (LEDCount == 0) {
    digitalWrite(L_PIN, LEDState);
    if (LEDState == HIGH) {
      LEDState = LOW;
    }
    else {
      LEDState = HIGH;
    }
    LEDCount = 10000;
  }
}

static void iBeamStateChangedToOpen(Context &context) {
#ifdef DEBUG
  Serial.print("State: ");
  Serial.print(stateAsString(context.state));
  Serial.print(" > ");
#endif
  switch (context.state) {
    case IDLE:
      context.state = O_STEP_1;
      break;
    case O_STEP_3:
      context.state = O_STEP_2;
      break;
    case I_STEP_1:
      context.state = I_STEP_2;
      break;
    default:
      context.state = ERROR;
      break;
  }
#ifdef DEBUG
  Serial.println(stateAsString(context.state));
#endif
}

static void iBeamStateChangedToClosed(Context &context) {
#ifdef DEBUG
  Serial.print("State: ");
  Serial.print(stateAsString(context.state));
  Serial.print(" > ");
#endif
  switch (context.state) {
    case O_STEP_1:
      context.state = IDLE;
      break;
    case O_STEP_2:
      context.state = O_STEP_3;
      break;
    case I_STEP_2:
      context.state = I_STEP_1;
      break;
    case I_STEP_3:
      context.iValid = true;
      context.state = IDLE;
      break;
    default:
      context.state = ERROR;
      break;
  }
#ifdef DEBUG
  Serial.println(stateAsString(context.state));
#endif
}

static void oBeamStateChangedToOpen(Context &context) {
#ifdef DEBUG
  Serial.print("State: ");
  Serial.print(stateAsString(context.state));
  Serial.print(" > ");
#endif
  switch (context.state) {
    case IDLE:
      context.state = I_STEP_1;
      break;
    case I_STEP_3:
      context.state = I_STEP_2;
      break;
    case O_STEP_1:
      context.state = O_STEP_2;
      break;
    default:
      context.state = ERROR;
      break;
  }
#ifdef DEBUG
  Serial.println(stateAsString(context.state));
#endif
}

static void oBeamStateChangedToClosed(Context &context) {
#ifdef DEBUG
  Serial.print("State: ");
  Serial.print(stateAsString(context.state));
  Serial.print(" > ");
#endif
  switch (context.state) {
    case I_STEP_1:
      context.state = IDLE;
      break;
    case I_STEP_2:
      context.state = I_STEP_3;
      break;
    case O_STEP_2:
      context.state = O_STEP_1;
      break;
    case O_STEP_3:
      context.oValid = true;
      context.state = IDLE;
      break;
    default:
      context.state = ERROR;
      break;
  }
#ifdef DEBUG
  Serial.println(stateAsString(context.state));
#endif
}

static int iValid(Context &context) {
  if (context.iValid) {
    context.iValid = false;
    return true;
  }
  return false;
}

static int oValid(Context &context) {
  if (context.oValid) {
    context.oValid = false;
    return true;
  }
  return false;
}

static String beamStateAsString(int state) {
  switch (state) {
    case OPEN:
      return "OPEN";
      break;
    case CLOSED:
      return "CLOSED";
      break;
    default:
      return "<unknown>";
      break;
  }
}

static String stateAsString(int state) {
  switch (state) {
    case IDLE:
      return "IDLE";
      break;
    case I_STEP_1:
      return "I_STEP_1";
      break;
    case I_STEP_2:
      return "I_STEP_2";
      break;
    case I_STEP_3:
      return "I_STEP_3";
      break;
    case O_STEP_1:
      return "O_STEP_1";
      break;
    case O_STEP_2:
      return "O_STEP_2";
      break;
    case O_STEP_3:
      return "O_STEP_3";
      break;
    case ERROR:
      return "ERROR";
      break;
    default:
      return "<unknown>";
      break;
  }
}

