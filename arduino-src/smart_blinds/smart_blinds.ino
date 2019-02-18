#define TEST_DEBUG

#include "smart_blinds.h"

#include <SoftwareSerial.h>

#ifdef TEST_DEBUG
#include "tests.h"
#endif  // TEST_DEBUG

#define STEPPER_A               2
#define STEPPER_B               3
#define STEPPER_C               4
#define STEPPER_D               5
#define RPM                     15
#define STEP_ANGLE              1.8
#define STEPS_PER_REVOLUTION    (360.0 / STEP_ANGLE)

#define RELAY_PIN               6

#define BUTTON_PIN              7

#define ESP_RX                  8
#define ESP_TX                  9

Stepper stepper(STEPS_PER_REVOLUTION, STEPPER_A, STEPPER_B, STEPPER_C, STEPPER_D);
Relay relay(RELAY_PIN, Relay::Mode::NORMALLY_OPEN);
ace_button::AceButton btn(BUTTON_PIN);
SoftwareSerial esp(ESP_RX, ESP_TX);

int stepperPos = STEPPER_POSITION_DEFAULT;
int stepperPosLowerLimit = STEPPER_POSITION_LOWER_LIMIT_DEFAULT;
int stepperPosUpperLimit = STEPPER_POSITION_UPPER_LIMIT_DEFAULT;

void setup()
{
    Serial.begin(9600);

    // TODO load position from eeprom
    // - if values found in eeprom, load stepper pos and lims
    // - if not then do nothing

    relay.begin();
    stepper.setSpeed(RPM);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    btn.setEventHandler(handleBtnEvent);

    relay.open();

    // TODO: esp checks
    // TODO: wps support
    esp.begin(9600);

#ifdef TEST_DEBUG
    Serial.println("Running tests...");
    test();
    Serial.println("Tests complete and successful");
#endif  // TEST_DEBUG
}

void loop()
{
#ifndef TEST_DEBUG
    btn.check();
#endif  // TEST_DEBUG

    // TODO: esp checks

}

void handleBtnEvent(ace_button::AceButton* /*button*/, uint8_t eventType, uint8_t /*state*/) {
    switch (eventType) {
    case ace_button::AceButton::kEventReleased:
        setStepperPos((stepperPos == stepperPosLowerLimit) ?
                      stepperPosUpperLimit : stepperPosLowerLimit);
        break;
    default:
        break;
    }
}

bool isPosInverted()
{
    return stepperPosUpperLimit > stepperPosLowerLimit;
}

bool isStepperCalibrated()
{
    return (stepperPos != STEPPER_POSITION_DEFAULT) &&
           (stepperPosLowerLimit != STEPPER_POSITION_LOWER_LIMIT_DEFAULT) &&
           (stepperPosUpperLimit != STEPPER_POSITION_UPPER_LIMIT_DEFAULT);
}

bool isPosOutOfBounds(int pos)
{
    bool retval;

    bool inverted = isPosInverted();
    if ((!inverted &&
         (stepperPosLowerLimit <= pos) &&
         (pos <= stepperPosUpperLimit)) ||
        (inverted &&
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

    if (true) { // TODO temp
        success = true;
        pos = stepperPos;
    }

    return success;
}

bool setStepperPos(int pos)
{
    bool success = false;

    if (relay.isClosed() && !isPosOutOfBounds(pos)) {
        success = true;

        int stepValue;

        if (isPosInverted()) {
            stepValue = stepperPos - pos;
        } else {
            stepValue = pos - stepperPos;
        } /* else {
            success = false;
        } */

        if (success) {
            stepper.step(stepValue);
            stepperPos = pos;
        }
    }

    return success;
}

bool getStepperPosLowerLimit(int& pos)
{
    bool success = false;

    if (true) { // TODO temp
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

    if (true) { // TODO temp
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

