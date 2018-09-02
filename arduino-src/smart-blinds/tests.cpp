#define __ASSERT_USE_STDERR
#include "tests.h"

#include <assert.h>
#include <Arduino.h>
#include "smart-blinds.h"

// TODO include arduino sketch header

#define TEST_STEPPER_POSITION_LOWER_LIMIT    0
#define TEST_STEPPER_POSITION_UPPER_LIMIT    100
#define TEST_STEPPER_POSITION_DEFAULT        ((TEST_STEPPER_POSITION_LOWER_LIMIT +    \
                                               TEST_STEPPER_POSITION_UPPER_LIMIT) /   \
                                              2)

static void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp)
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

static void uncalibrate()
{
    stepperPos = STEPPER_POSITION_UNDEFINED;
    stepperPosLowerLimit = STEPPER_POSITION_UNDEFINED;
    stepperPosUpperLimit = STEPPER_POSITION_UNDEFINED;
}

static void position_and_calibrate_with_normal_scale(int lo, int pos, int hi)
{
    stepperPos = pos;
    stepperPosLowerLimit = lo;
    stepperPosUpperLimit = hi;
}

static void position_and_calibrate_with_inverted_scale(int lo, int pos, int hi)
{
    stepperPos = pos;
    stepperPosLowerLimit = hi;
    stepperPosUpperLimit = lo;
}

void test()
{
    int pos;

    /* test predefined position values */
    assert(STEP_INCREMENT_SIZE > 0);
    assert(TEST_STEPPER_POSITION_LOWER_LIMIT != STEPPER_POSITION_UNDEFINED);
    assert(TEST_STEPPER_POSITION_UPPER_LIMIT != STEPPER_POSITION_UNDEFINED);
    assert(TEST_STEPPER_POSITION_DEFAULT != STEPPER_POSITION_UNDEFINED);
    assert(TEST_STEPPER_POSITION_LOWER_LIMIT != TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(abs(TEST_STEPPER_POSITION_DEFAULT - TEST_STEPPER_POSITION_LOWER_LIMIT) >= STEP_INCREMENT_SIZE);
    assert(abs(TEST_STEPPER_POSITION_UPPER_LIMIT - TEST_STEPPER_POSITION_DEFAULT) >= STEP_INCREMENT_SIZE);

    /* getStepperPositionScaleType() */

    uncalibrate();
    assert(getStepperPositionScaleType() == UNDEFINED);

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(getStepperPositionScaleType() == NORMAL);

    position_and_calibrate_with_inverted_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                               TEST_STEPPER_POSITION_DEFAULT,
                                               TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(getStepperPositionScaleType() == INVERTED);

    /* isStepperCalibrated() */

    uncalibrate();
    assert(!isStepperCalibrated());

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(isStepperCalibrated());

    position_and_calibrate_with_inverted_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                               TEST_STEPPER_POSITION_DEFAULT,
                                               TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(isStepperCalibrated());

    /* isPosOutOfBounds() */

    uncalibrate();
    assert(isPosOutOfBounds(TEST_STEPPER_POSITION_DEFAULT));

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(!isPosOutOfBounds(TEST_STEPPER_POSITION_DEFAULT));
    assert(isPosOutOfBounds(TEST_STEPPER_POSITION_UPPER_LIMIT + STEP_INCREMENT_SIZE));
    assert(isPosOutOfBounds(TEST_STEPPER_POSITION_LOWER_LIMIT - STEP_INCREMENT_SIZE));

    position_and_calibrate_with_inverted_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                               TEST_STEPPER_POSITION_DEFAULT,
                                               TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(!isPosOutOfBounds(TEST_STEPPER_POSITION_DEFAULT));
    assert(isPosOutOfBounds(TEST_STEPPER_POSITION_UPPER_LIMIT + STEP_INCREMENT_SIZE));
    assert(isPosOutOfBounds(TEST_STEPPER_POSITION_LOWER_LIMIT - STEP_INCREMENT_SIZE));

    /* getStepperPos() */

    uncalibrate();
    assert(!getStepperPos(pos));

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(getStepperPos(pos));
    assert(pos == stepperPos);

    /* setStepperPos(), incrementStepperPos(), decrementStepperPos() */

    // fail when uncalibrated
    uncalibrate();
    relay.close();
    assert(!setStepperPos(TEST_STEPPER_POSITION_DEFAULT));
    assert(!incrementStepperPos());
    assert(!decrementStepperPos());

    // fail when stepper off (relay open)
    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    relay.open();
    assert(!setStepperPos(TEST_STEPPER_POSITION_LOWER_LIMIT));
    assert(!incrementStepperPos());
    assert(!decrementStepperPos());

    // fail when calibrated, but position of-of-bounds
    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    relay.close();
    assert(!setStepperPos(TEST_STEPPER_POSITION_UPPER_LIMIT + STEP_INCREMENT_SIZE));
    setStepperPos(TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(!incrementStepperPos());
    setStepperPos(TEST_STEPPER_POSITION_LOWER_LIMIT);
    assert(!decrementStepperPos());

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    relay.close();
    // success on setting position
    assert(setStepperPos(TEST_STEPPER_POSITION_LOWER_LIMIT));
    assert(stepperPos == TEST_STEPPER_POSITION_LOWER_LIMIT);
    // success on incrementing position
    setStepperPos(TEST_STEPPER_POSITION_DEFAULT);
    assert(incrementStepperPos());
    assert(stepperPos == TEST_STEPPER_POSITION_DEFAULT + STEP_INCREMENT_SIZE);
    // success on decrementing position
    setStepperPos(TEST_STEPPER_POSITION_DEFAULT);
    assert(decrementStepperPos());
    assert(stepperPos == TEST_STEPPER_POSITION_DEFAULT - STEP_INCREMENT_SIZE);

    /* getStepperPosLowerLimit(), getStepperPosUpperLimit() */

    uncalibrate();
    assert(!getStepperPosLowerLimit(pos));
    assert(!getStepperPosUpperLimit(pos));

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(getStepperPosLowerLimit(pos));
    assert(pos == TEST_STEPPER_POSITION_LOWER_LIMIT);
    assert(getStepperPosUpperLimit(pos));
    assert(pos == TEST_STEPPER_POSITION_UPPER_LIMIT);

    /* setStepperPosLowerLimit(), setStepperPosUpperLimit() */

    relay.close();
    assert(!setStepperPosUpperLimit(TEST_STEPPER_POSITION_DEFAULT));
    assert(!setStepperPosLowerLimit(TEST_STEPPER_POSITION_DEFAULT));

    relay.open();
    assert(setStepperPosUpperLimit(TEST_STEPPER_POSITION_DEFAULT));
    assert(setStepperPosLowerLimit(TEST_STEPPER_POSITION_DEFAULT));
}

