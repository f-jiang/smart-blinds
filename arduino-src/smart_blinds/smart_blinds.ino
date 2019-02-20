#define TEST_DEBUG

#include "smart_blinds.h"

#include <stdlib.h>
#include <SoftwareSerial.h>
#ifdef TEST_DEBUG
#include "tests.h"
#endif  // TEST_DEBUG
#include "wear_leveled_eeprom_object.h"

#define SERIAL_BAUD_RATE            9600
#define SOFTWARE_SERIAL_BAUD_RATE   9600

#define STEPPER_A                   2
#define STEPPER_B                   3
#define STEPPER_C                   4
#define STEPPER_D                   5
#define RPM                         15
#define STEP_ANGLE                  1.8
#define STEPS_PER_REVOLUTION        (360.0 / STEP_ANGLE)

#define RELAY_PIN                   6

#define BUTTON_PIN                  7

#define ESP_RX                      8
#define ESP_TX                      9

#define COMMAND_TILT                't'
#define COMMAND_CALIBRATE_HIGH      'h'
#define COMMAND_CALIBRATE_LOW       'l'

#define STR_BUF_LEN                 10

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
    Serial.begin(SERIAL_BAUD_RATE);

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

    esp.begin(SOFTWARE_SERIAL_BAUD_RATE);

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

    if (esp.available() > 0) {
        String data;
        while (esp.available() > 0) {
            data += (char) esp.read();
        }

        char command = data.charAt(0);
        data.remove(0, 1);
        int value = data.toInt();
        stepper_pos_t new_value;
        char buf[STR_BUF_LEN];

        switch (command) {
            case COMMAND_TILT:
                relay.close();
                setStepperPos((stepper_pos_t) value);
                relay.open();

                getStepperPos(new_value);
                sprintf(buf, "%d", new_value);
                esp.write(COMMAND_TILT);
                esp.write(buf);
                break;
            case COMMAND_CALIBRATE_HIGH:
                relay.open();
                setStepperPosUpperLimit((stepper_pos_t) value);

                getStepperPosUpperLimit(new_value);
                sprintf(buf, "%d", new_value);
                esp.write(COMMAND_CALIBRATE_HIGH);
                esp.write(buf);
                break;
            case COMMAND_CALIBRATE_LOW:
                relay.open();
                setStepperPosLowerLimit((stepper_pos_t) value);

                getStepperPosLowerLimit(new_value);
                sprintf(buf, "%d", new_value);
                esp.write(COMMAND_CALIBRATE_LOW);
                esp.write(buf);
                break;
            default:
                break;
        }
    }

    delay(100);
#endif  // TEST_DEBUG
}

void handleBtnEvent(ace_button::AceButton* /*button*/, uint8_t eventType, uint8_t /*state*/)
{
    switch (eventType) {
        case ace_button::AceButton::kEventReleased:
        {
            stepper_pos_t pos, lowerLimit;
            stepperPos.get(pos);
            stepperPos.get(lowerLimit);

            relay.close();
            setStepperPos((pos == lowerLimit) ?
                          stepperPosUpperLimit.get(eepromValue) : stepperPosLowerLimit.get(eepromValue));
            relay.open();

            stepper_pos_t new_value;
            char buf[STR_BUF_LEN];
            getStepperPos(new_value);
            sprintf(buf, "%d", new_value);
            esp.write(COMMAND_TILT);
            esp.write(buf);

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

