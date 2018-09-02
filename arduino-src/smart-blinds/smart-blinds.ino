#define TEST_DEBUG

#include "smart-blinds.h"

#ifdef TEST_DEBUG
#include "tests.h"
#endif  // TEST_DEBUG

#define STEPPER_A               8
#define STEPPER_B               9
#define STEPPER_C               10
#define STEPPER_D               11
#define RPM                     15
#define STEP_ANGLE              1.8
#define STEPS_PER_REVOLUTION    (360.0 / STEP_ANGLE)

#define RELAY_PIN               6

#define BUTTON_PIN              7

Stepper stepper(STEPS_PER_REVOLUTION, STEPPER_A, STEPPER_B, STEPPER_C, STEPPER_D);
Relay relay(RELAY_PIN, Relay::Mode::NORMALLY_OPEN);
int stepperPos = STEPPER_POSITION_UNDEFINED;
int stepperPosLowerLimit = STEPPER_POSITION_UNDEFINED;
int stepperPosUpperLimit = STEPPER_POSITION_UNDEFINED;

void setup()
{
    Serial.begin(9600);

    // TODO load position from eeprom

    relay.begin();
    stepper.setSpeed(RPM);

    relay.open();

#ifdef TEST_DEBUG
    Serial.println("Running tests...");
    test();
    Serial.println("Tests complete and successful");
#endif  // TEST_DEBUG
}

void loop()
{
}

StepperPositionScaleType getStepperPositionScaleType()
{
    StepperPositionScaleType retval;

    if (!isStepperCalibrated()) {
        retval = UNDEFINED;
    } else if (stepperPosLowerLimit <= stepperPosUpperLimit) {
        retval = NORMAL;
    } else {
        retval = INVERTED;
    }

    return retval;
}

bool isStepperCalibrated()
{
    return (stepperPos != STEPPER_POSITION_UNDEFINED) &&
           (stepperPosLowerLimit != STEPPER_POSITION_UNDEFINED) &&
           (stepperPosUpperLimit != STEPPER_POSITION_UNDEFINED);
}

bool isPosOutOfBounds(int pos)
{
    bool retval;

    StepperPositionScaleType scaleType = getStepperPositionScaleType();
    if (!isStepperCalibrated()) {
        retval = true;
    } else if (((scaleType == NORMAL) &&
                (stepperPosLowerLimit <= pos) &&
                (pos <= stepperPosUpperLimit)) ||
               ((scaleType == INVERTED) &&
                (stepperPosUpperLimit <= pos) &&
                (pos <= stepperPosLowerLimit))) {
        retval = false;
    } else {
        retval = true;
    }

    return retval;
}

bool getStepperPos(int& pos)
{
    bool success = false;

    if (isStepperCalibrated()) {
        success = true;
        pos = stepperPos;
    }

    return success;
}

bool setStepperPos(int pos)
{
    bool success = false;

    if (isStepperCalibrated() && relay.isClosed() && !isPosOutOfBounds(pos)) {
        success = true;

        int stepValue;
        StepperPositionScaleType scaleType = getStepperPositionScaleType();

        if (scaleType == NORMAL) {
            stepValue = pos - stepperPos;
        } else if (scaleType == INVERTED) {
            stepValue = stepperPos - pos;
        } else {
            success = false;
        }

        if (success) {
            stepper.step(stepValue);
            stepperPos = pos;
        }
    }

    return success;
}

bool incrementStepperPos()
{
    bool success = true;

    int newPos;
    StepperPositionScaleType scaleType = getStepperPositionScaleType();

    if (scaleType == NORMAL) {
        newPos = stepperPos + STEP_INCREMENT_SIZE;
    } else if (scaleType == INVERTED) {
        newPos = stepperPos - STEP_INCREMENT_SIZE;
    } else {
        success = false;
    }

    if (success) {
        success = setStepperPos(newPos);
    }

    return success;
}

bool decrementStepperPos()
{
    bool success = true;

    int newPos;
    StepperPositionScaleType scaleType = getStepperPositionScaleType();

    if (scaleType == NORMAL) {
        newPos = stepperPos - STEP_INCREMENT_SIZE;
    } else if (scaleType == INVERTED) {
        newPos = stepperPos + STEP_INCREMENT_SIZE;
    } else {
        success = false;
    }

    if (success) {
        success = setStepperPos(newPos);
    }

    return success;
}

bool getStepperPosLowerLimit(int& pos)
{
    bool success = false;

    if (isStepperCalibrated()) {
        success = true;
        pos = stepperPosLowerLimit;
    }

    return success;
}

// TODO enforce lower != upper?
bool setStepperPosLowerLimit(int pos)
{
    bool success = false;

    if (!relay.isClosed()) {
        success = true;
        stepperPosLowerLimit = pos;
    }

    return success;
}

bool getStepperPosUpperLimit(int& pos)
{
    bool success = false;

    if (isStepperCalibrated()) {
        success = true;
        pos = stepperPosUpperLimit;
    }

    return success;
}

// TODO enforce lower != upper?
bool setStepperPosUpperLimit(int pos)
{
    bool success = false;

    if (!relay.isClosed()) {
        success = true;
        stepperPosUpperLimit = pos;
    }

    return success;
}

