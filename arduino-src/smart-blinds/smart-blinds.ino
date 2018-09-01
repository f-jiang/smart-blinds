#define TEST

#ifdef TEST
#define __ASSERT_USE_STDERR
#include <assert.h>
#endif  // TEST
#include <limits.h>

#include <Stepper.h>
#include <Relay.h>

#define STEPPER_A                       8
#define STEPPER_B                       9
#define STEPPER_C                       10
#define STEPPER_D                       11
#define RPM                             30
#define STEP_ANGLE                      1.8
#define STEP_INCREMENT_SIZE             1
#define STEPS_PER_REVOLUTION            (360.0 / STEP_ANGLE)
#define POSITION_UNDEFINED              (INT_MAX - 1)

#define RELAY_PIN                       6

#define BUTTON_PIN                      7

#ifdef TEST
#define TEST_STEPPER_POS_LOWER_LIMIT    0
#define TEST_STEPPER_POS_UPPER_LIMIT    10
#define TEST_STEPPER_DEFAULT_POS        ((TEST_STEPPER_POS_LOWER_LIMIT +    \
                                          TEST_STEPPER_POS_UPPER_LIMIT) /   \
                                         2)
#endif  // TEST

enum StepperPositionScaleType {
    NORMAL,
    INVERTED,
    UNDEFINED
};

Stepper stepper(STEPS_PER_REVOLUTION, STEPPER_A, STEPPER_B, STEPPER_C, STEPPER_D);
Relay relay(RELAY_PIN, Relay::Mode::NORMALLY_OPEN);
int stepperPos = POSITION_UNDEFINED;
int stepperPosLowerLimit = POSITION_UNDEFINED;
int stepperPosUpperLimit = POSITION_UNDEFINED;

StepperPositionScaleType getStepperPositionScaleType();
bool isStepperCalibrated();
bool isPosOutOfBounds(int pos);
bool getStepperPos(int& pos);
bool setStepperPos(int pos);
bool incrementStepperPos();
bool decrementStepperPos();
bool getStepperPosLowerLimit(int& pos);
bool setStepperPosLowerLimit(int pos);
bool getStepperPosUpperLimit(int& pos);
bool setStepperPosUpperLimit(int pos);
#ifdef TEST
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp);
void uncalibrate();
void position_and_calibrate_with_normal_scale(int lo, int pos, int hi);
void position_and_calibrate_with_inverted_scale(int lo, int pos, int hi);
void test();
#endif  // TEST

void setup() {
    Serial.begin(9600);

    // TODO load position from eeprom

    relay.begin();
    stepper.setSpeed(RPM);

    relay.close();

#ifdef TEST
    test();
#endif  // TEST
}

void loop() {
}

StepperPositionScaleType getStepperPositionScaleType() {
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

bool isStepperCalibrated() {
    return (stepperPos != POSITION_UNDEFINED) &&
           (stepperPosLowerLimit != POSITION_UNDEFINED) &&
           (stepperPosUpperLimit != POSITION_UNDEFINED);
}

bool isPosOutOfBounds(int pos) {
    bool retval;

    StepperPositionScaleType scaleType = getStepperPositionScaleType();
    if (!isStepperCalibrated()) {
        retval = true;
    } else if (((scaleType == NORMAL) &&
                (stepperPosLowerLimit <= stepperPos) &&
                (stepperPos <= stepperPosUpperLimit)) ||
               ((scaleType == INVERTED) &&
                (stepperPosUpperLimit <= stepperPos) &&
                (stepperPos <= stepperPosLowerLimit))) {
        retval = false;
    } else {
        retval = true;
    }

    return retval;
}

bool getStepperPos(int& pos) {
    bool success = false;

    if (isStepperCalibrated()) {
        success = true;
        pos = stepperPos;
    }

    return success;
}

bool setStepperPos(int pos) {
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

bool incrementStepperPos() {
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

bool decrementStepperPos() {
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

bool getStepperPosLowerLimit(int& pos) {
    bool success = false;

    if (isStepperCalibrated()) {
        success = true;
        pos = stepperPosLowerLimit;
    }

    return success;
}

// TODO enforce lower != upper?
bool setStepperPosLowerLimit(int pos) {
    bool success = false;

    if (!relay.isClosed()) {
        success = true;
        stepperPosLowerLimit = pos;
    }

    return success;
}

bool getStepperPosUpperLimit(int& pos) {
    bool success = false;

    if (isStepperCalibrated()) {
        success = true;
        pos = stepperPosUpperLimit;
    }

    return success;
}

// TODO enforce lower != upper?
bool setStepperPosUpperLimit(int pos) {
    bool success = false;

    if (!relay.isClosed()) {
        success = true;
        stepperPosUpperLimit = pos;
    }

    return success;
}

#ifdef TEST
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp)
{
    Serial.print("Assertion failed: (");
    Serial.print(__sexp);
    Serial.print("), ");
    if (__func != NULL) {
      Serial.print("function ");
      Serial.print(__func);
      Serial.print(", ");
    }
    Serial.print("file ");
    Serial.print(__file);
    Serial.print(", ");
    Serial.print("line ");
    Serial.println(__lineno, DEC);
    Serial.print("\n");

    Serial.flush();
    // abort program execution.
    abort();
}

void uncalibrate() {
    stepperPos = POSITION_UNDEFINED;
    stepperPosLowerLimit = POSITION_UNDEFINED;
    stepperPosUpperLimit = POSITION_UNDEFINED;
}

void position_and_calibrate_with_normal_scale(int lo, int pos, int hi) {
    stepperPos = pos;
    stepperPosLowerLimit = lo;
    stepperPosUpperLimit = hi;
}

void position_and_calibrate_with_inverted_scale(int lo, int pos, int hi) {
    stepperPos = pos;
    stepperPosLowerLimit = hi;
    stepperPosUpperLimit = lo;
}

void test() {
    int pos;

    /* test predefined position values */
    assert(TEST_STEPPER_POS_LOWER_LIMIT != TEST_STEPPER_POS_UPPER_LIMIT);
    assert(abs(TEST_STEPPER_DEFAULT_POS - TEST_STEPPER_POS_LOWER_LIMIT) >= STEP_INCREMENT_SIZE);
    assert(abs(TEST_STEPPER_POS_UPPER_LIMIT - TEST_STEPPER_DEFAULT_POS) >= STEP_INCREMENT_SIZE);

    /* getStepperPositionScaleType() */

    uncalibrate();
    assert(getStepperPositionScaleType() == UNDEFINED);

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                             TEST_STEPPER_DEFAULT_POS,
                                             TEST_STEPPER_POS_UPPER_LIMIT);
    assert(getStepperPositionScaleType() == NORMAL);

    position_and_calibrate_with_inverted_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                               TEST_STEPPER_DEFAULT_POS,
                                               TEST_STEPPER_POS_UPPER_LIMIT);
    assert(getStepperPositionScaleType() == INVERTED);

    /* isStepperCalibrated() */

    uncalibrate();
    assert(!isStepperCalibrated());

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                             TEST_STEPPER_DEFAULT_POS,
                                             TEST_STEPPER_POS_UPPER_LIMIT);
    assert(isStepperCalibrated());

    position_and_calibrate_with_inverted_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                               TEST_STEPPER_DEFAULT_POS,
                                               TEST_STEPPER_POS_UPPER_LIMIT);
    assert(isStepperCalibrated());

    /* isPosOutOfBounds() */

    uncalibrate();
    assert(isPosOutOfBounds(TEST_STEPPER_DEFAULT_POS));

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                             TEST_STEPPER_DEFAULT_POS,
                                             TEST_STEPPER_POS_UPPER_LIMIT);
    assert(!isPosOutOfBounds(TEST_STEPPER_DEFAULT_POS));
    assert(isPosOutOfBounds(TEST_STEPPER_POS_UPPER_LIMIT + STEP_INCREMENT_SIZE));
    assert(isPosOutOfBounds(TEST_STEPPER_POS_UPPER_LIMIT - STEP_INCREMENT_SIZE));

    position_and_calibrate_with_inverted_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                               TEST_STEPPER_DEFAULT_POS,
                                               TEST_STEPPER_POS_UPPER_LIMIT);
    assert(!isPosOutOfBounds(TEST_STEPPER_DEFAULT_POS));
    assert(isPosOutOfBounds(TEST_STEPPER_POS_UPPER_LIMIT + STEP_INCREMENT_SIZE));
    assert(isPosOutOfBounds(TEST_STEPPER_POS_UPPER_LIMIT - STEP_INCREMENT_SIZE));

    /* getStepperPos() */

    uncalibrate();
    assert(!getStepperPos(pos));

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                             TEST_STEPPER_DEFAULT_POS,
                                             TEST_STEPPER_POS_UPPER_LIMIT);
    assert(getStepperPos(pos));
    assert(pos == stepperPos);

    /* setStepperPos(), incrementStepperPos(), decrementStepperPos() */

    // fail when uncalibrated
    uncalibrate();
    relay.close();
    assert(!setStepperPos(TEST_STEPPER_DEFAULT_POS));
    assert(!incrementStepperPos());
    assert(!decrementStepperPos());

    // fail when stepper off (relay open)
    position_and_calibrate_with_normal_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                             TEST_STEPPER_DEFAULT_POS,
                                             TEST_STEPPER_POS_UPPER_LIMIT);
    relay.open();
    assert(!setStepperPos(TEST_STEPPER_DEFAULT_POS));
    assert(!incrementStepperPos());
    assert(!decrementStepperPos());

    // fail when calibrated, but position of-of-bounds
    position_and_calibrate_with_normal_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                             TEST_STEPPER_DEFAULT_POS,
                                             TEST_STEPPER_POS_UPPER_LIMIT);
    relay.close();
    assert(!setStepperPos(TEST_STEPPER_POS_UPPER_LIMIT + STEP_INCREMENT_SIZE));
    stepperPos = TEST_STEPPER_POS_UPPER_LIMIT;
    assert(!incrementStepperPos());
    stepperPos = TEST_STEPPER_POS_LOWER_LIMIT;
    assert(!decrementStepperPos());

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                             TEST_STEPPER_DEFAULT_POS,
                                             TEST_STEPPER_POS_UPPER_LIMIT);
    relay.close();
    // success on setting position
    assert(setStepperPos(TEST_STEPPER_DEFAULT_POS - STEP_INCREMENT_SIZE));
    assert(stepperPos == TEST_STEPPER_DEFAULT_POS - STEP_INCREMENT_SIZE);
    // success on incrementing position
    stepperPos = TEST_STEPPER_DEFAULT_POS;
    assert(incrementStepperPos());
    assert(stepperPos == TEST_STEPPER_DEFAULT_POS + STEP_INCREMENT_SIZE);
    // success on decrementing position
    stepperPos = TEST_STEPPER_DEFAULT_POS;
    assert(decrementStepperPos());
    assert(stepperPos == TEST_STEPPER_DEFAULT_POS - STEP_INCREMENT_SIZE);

    /* getStepperPosLowerLimit(), getStepperPosUpperLimit() */

    uncalibrate();
    assert(!getStepperPosLowerLimit(pos));
    assert(!getStepperPosUpperLimit(pos));

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POS_LOWER_LIMIT,
                                             TEST_STEPPER_DEFAULT_POS,
                                             TEST_STEPPER_POS_UPPER_LIMIT);
    assert(getStepperPosLowerLimit(pos));
    assert(pos == TEST_STEPPER_POS_LOWER_LIMIT);
    assert(getStepperPosUpperLimit(pos));
    assert(pos == TEST_STEPPER_POS_UPPER_LIMIT);

    /* setStepperPosLowerLimit(), setStepperPosUpperLimit() */

    relay.close();
    assert(!setStepperPosUpperLimit(TEST_STEPPER_DEFAULT_POS));
    assert(!setStepperPosLowerLimit(TEST_STEPPER_DEFAULT_POS));

    relay.open();
    assert(setStepperPosUpperLimit(TEST_STEPPER_DEFAULT_POS));
    assert(setStepperPosLowerLimit(TEST_STEPPER_DEFAULT_POS));
}
#endif  // TEST

