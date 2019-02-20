/* #define TEST */
#define DEBUG

#include "smart_blinds.h"

#include <stdlib.h>
#include <SoftwareSerial.h>
#ifdef TEST
#include "tests.h"
#endif
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
#ifdef DEBUG
        Serial.println("no position or limits in storage; using defaults instead");
#endif
        eepromValue = STEPPER_POSITION_DEFAULT;
        stepperPos.put(eepromValue);
        eepromValue = STEPPER_POSITION_LOWER_LIMIT_DEFAULT;
        stepperPosLowerLimit.put(eepromValue);
        eepromValue = STEPPER_POSITION_UPPER_LIMIT_DEFAULT;
        stepperPosUpperLimit.put(eepromValue);
#ifndef DEBUG
    }
#else
    } else {
        stepper_pos_t pos, lower_limit, upper_limit;
        stepperPos.get(pos);
        stepperPosLowerLimit.get(lower_limit);
        stepperPosUpperLimit.get(upper_limit);

        Serial.print("found following in storage: position ");
        Serial.print(pos);
        Serial.print(", lower limit ");
        Serial.print(lower_limit);
        Serial.print(", upper limit ");
        Serial.println(upper_limit);
    }
#endif

    relay.begin();
    stepper.setSpeed(RPM);
#ifdef DEBUG
    Serial.print("stepper speed set to ");
    Serial.print(RPM);
    Serial.println(" rpm");
#endif

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    btn.setEventHandler(handleBtnEvent);

    relay.open();

    esp.begin(SOFTWARE_SERIAL_BAUD_RATE);

#ifdef TEST
    Serial.println("Running tests...");
    test();
    Serial.println("Tests complete and successful");
#endif
}

void loop()
{
#ifndef TEST
    btn.check();

    if (esp.available() > 0) {
        String data;
        while (esp.available() > 0) {
            data += (char) esp.read();
        }

#ifdef DEBUG
        Serial.print("received data from serial: ");
        Serial.println(data);
#endif

        char command = data.charAt(0);
        data.remove(0, 1);
        int value = data.toInt();
        stepper_pos_t new_value;
        char buf[STR_BUF_LEN];

#ifdef DEBUG
        Serial.print("values extracted from incoming serial data: command ");
        Serial.print(command);
        Serial.print(", value ");
        Serial.println(value);
#endif

        switch (command) {
            case COMMAND_TILT:
                relay.close();
                setStepperPos((stepper_pos_t) value);
                relay.open();

                getStepperPos(new_value);
                sprintf(buf, "%d", new_value);
                esp.write(COMMAND_TILT);
                esp.write(buf);

#ifdef DEBUG
                Serial.print("new stepper position: ");
                Serial.println(new_value);
#endif
                break;
            case COMMAND_CALIBRATE_HIGH:
                relay.open();
                setStepperPosUpperLimit((stepper_pos_t) value);

                getStepperPosUpperLimit(new_value);
                sprintf(buf, "%d", new_value);
                esp.write(COMMAND_CALIBRATE_HIGH);
                esp.write(buf);

#ifdef DEBUG
                Serial.print("new stepper upper limit: ");
                Serial.println(new_value);
#endif
                break;
            case COMMAND_CALIBRATE_LOW:
                relay.open();
                setStepperPosLowerLimit((stepper_pos_t) value);

                getStepperPosLowerLimit(new_value);
                sprintf(buf, "%d", new_value);
                esp.write(COMMAND_CALIBRATE_LOW);
                esp.write(buf);

#ifdef DEBUG
                Serial.print("new stepper lower limit: ");
                Serial.println(new_value);
#endif
                break;
            default:
#ifdef DEBUG
                Serial.println("not a valid command");
#endif
                break;
        }
    }

    delay(100);
#endif
}

void handleBtnEvent(ace_button::AceButton* /*button*/, uint8_t eventType, uint8_t /*state*/)
{
    switch (eventType) {
        case ace_button::AceButton::kEventReleased:
        {
#ifdef DEBUG
            Serial.println("button press detected");
#endif

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

#ifdef DEBUG
            Serial.print("new stepper position: ");
            Serial.print(new_value);
            Serial.print(" (");
            Serial.print((new_value == lowerLimit) ? "lower" : "upper");
            Serial.println(" limit)");
#endif
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
            stepperPos.put(pos);
        }
#ifndef DEBUG
    }
#else
        Serial.print("set stepper position to ");
        Serial.println(pos);
    } else if (!relay.isClosed()) {
        Serial.println("setting stepper position failed because relay is open");
    } else {
        Serial.println("setting stepper position failed because new position is out-of-bounds");
    }
#endif

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

