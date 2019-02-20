#define TEST_DEBUG

#include "smart_blinds.h"

#include <SoftwareSerial.h>
#ifdef TEST_DEBUG
#include "tests.h"
#endif  // TEST_DEBUG
#include "wear_leveled_eeprom_object.h"

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

size_t circularQueueItemCount = EEPROM_SIZE_BYTES /
                                (3 * WearLeveledEepromObject<stepper_pos_t>::circularQueueItemSize());
WearLeveledEepromObject<stepper_pos_t> stepperPos(0, circularQueueItemCount);
WearLeveledEepromObject<stepper_pos_t> stepperPosLowerLimit(EEPROM_SIZE_BYTES / 3, circularQueueItemCount);
WearLeveledEepromObject<stepper_pos_t> stepperPosUpperLimit(2 * EEPROM_SIZE_BYTES / 3, circularQueueItemCount);
stepper_pos_t eepromValue;

void setup()
{
    Serial.begin(9600);

    if (stepperPos.get(eepromValue) == 0xFFFF &&
        stepperPosLowerLimit.get(eepromValue) == 0xFFFF &&
        stepperPosUpperLimit.get(eepromValue) == 0xFFFF)
    {
        eepromValue = STEPPER_POSITION_DEFAULT;
        stepperPos.put(eepromValue);
        eepromValue = STEPPER_POSITION_LOWER_LIMIT_DEFAULT;
        stepperPosLowerLimit.put(eepromValue);
        eepromValue = STEPPER_POSITION_UPPER_LIMIT_DEFAULT;
        stepperPosUpperLimit.put(eepromValue);
    }

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
    {
        stepper_pos_t pos, lowerLimit;
        stepperPos.get(pos);
        stepperPos.get(lowerLimit);

        setStepperPos((pos == lowerLimit) ?
                      stepperPosUpperLimit.get(eepromValue) : stepperPosLowerLimit.get(eepromValue));
        // TODO report new pos using getStepperPos and esp.write
        break;
    }
    default:
        break;
    }
}

bool isPosInverted()
{
    return stepperPosUpperLimit.get(eepromValue) < stepperPosLowerLimit.get(eepromValue);
}

bool isStepperCalibrated()
{
    return (stepperPos.get(eepromValue) != STEPPER_POSITION_DEFAULT) &&
           (stepperPosLowerLimit.get(eepromValue) != STEPPER_POSITION_LOWER_LIMIT_DEFAULT) &&
           (stepperPosUpperLimit.get(eepromValue) != STEPPER_POSITION_UPPER_LIMIT_DEFAULT);
}

bool isPosOutOfBounds(stepper_pos_t pos)
{
    bool retval;

    bool inverted = isPosInverted();
    if ((!inverted &&
         (stepperPosLowerLimit.get(eepromValue) <= pos) &&
         (pos <= stepperPosUpperLimit.get(eepromValue))) ||
        (inverted &&
         (stepperPosUpperLimit.get(eepromValue) <= pos) &&
         (pos <= stepperPosLowerLimit.get(eepromValue)))) {
        retval = false;
    } else {
        retval = true;
    }

    return retval;
}

bool getStepperPos(stepper_pos_t& pos)
{
    bool success = false;

    if (true) { // TODO temp
        success = true;
        stepperPos.get(pos);
    }

    return success;
}

bool setStepperPos(stepper_pos_t pos)
{
    bool success = false;

    if (relay.isClosed() && !isPosOutOfBounds(pos)) {
        success = true;

        stepper_pos_t stepValue;

        if (isPosInverted()) {
            stepValue = stepperPos.get(eepromValue) - pos;
        } else {
            stepValue = pos - stepperPos.get(eepromValue);
        } /* else {
            success = false;
        } */

        if (success) {
            stepper.step(stepValue);
            stepperPos.get(pos);
        }
    }

    return success;
}

bool getStepperPosLowerLimit(stepper_pos_t& pos)
{
    bool success = false;

    if (true) { // TODO temp
        success = true;
        stepperPosLowerLimit.get(pos);
    }

    return success;
}

// TODO enforce lower != upper?
bool setStepperPosLowerLimit(stepper_pos_t pos)
{
    bool success = false;

    if (!relay.isClosed()) {
        success = true;
        stepperPosLowerLimit.put(pos);
    }

    return success;
}

bool getStepperPosUpperLimit(stepper_pos_t& pos)
{
    bool success = false;

    if (true) { // TODO temp
        success = true;
        stepperPosUpperLimit.get(pos);
    }

    return success;
}

// TODO enforce lower != upper?
bool setStepperPosUpperLimit(stepper_pos_t pos)
{
    bool success = false;

    if (!relay.isClosed()) {
        success = true;
        stepperPosUpperLimit.put(pos);
    }

    return success;
}

