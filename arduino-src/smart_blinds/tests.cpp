#define __ASSERT_USE_STDERR
#include "tests.h"

#include <assert.h>
#include <stdint.h>

#include <Arduino.h>

#include "smart_blinds.h"
#define private public
#include "wear_leveled_eeprom_object.h"
#undef private

#define TEST_STEPPER_POSITION_LOWER_LIMIT              0
#define TEST_STEPPER_POSITION_UPPER_LIMIT              100
#define TEST_STEPPER_POSITION_DEFAULT                  ((TEST_STEPPER_POSITION_LOWER_LIMIT +    \
                                                         TEST_STEPPER_POSITION_UPPER_LIMIT) /   \
                                                        2)
#define TEST_EEPROM_START_ADDR                         0
#define TEST_EEPROM_QUEUE_ITEM_COUNT                   3
#define TEST_EEPROM_STORED_VALUE                       5
#define TEST_TRUNCATED_EEPROM_START_ADDR               (EEPROM_SIZE_BYTES - 10)
#define TEST_TRUNCATED_EEPROM_QUEUE_ITEM_COUNT         100
#define TEST_TRUNCATED_EEPROM_ACTUAL_QUEUE_ITEM_COUNT  1

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
    stepperPos = STEPPER_POSITION_DEFAULT;
    stepperPosLowerLimit = STEPPER_POSITION_LOWER_LIMIT_DEFAULT;
    stepperPosUpperLimit = STEPPER_POSITION_UPPER_LIMIT_DEFAULT;
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

static void test_smart_blinds()
{
    int pos;

    /* test predefined position values */
    assert(STEP_INCREMENT_SIZE > 0);
    assert(TEST_STEPPER_POSITION_LOWER_LIMIT != TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(TEST_STEPPER_POSITION_DEFAULT - TEST_STEPPER_POSITION_LOWER_LIMIT >= STEP_INCREMENT_SIZE);
    assert(TEST_STEPPER_POSITION_UPPER_LIMIT - TEST_STEPPER_POSITION_DEFAULT >= STEP_INCREMENT_SIZE);

    /* getStepperPositionScaleType() */

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

    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    assert(getStepperPos(pos));
    assert(pos == stepperPos);

    /* setStepperPos(), incrementStepperPos(), decrementStepperPos() */

    // fail when stepper off (relay open)
    position_and_calibrate_with_normal_scale(TEST_STEPPER_POSITION_LOWER_LIMIT,
                                             TEST_STEPPER_POSITION_DEFAULT,
                                             TEST_STEPPER_POSITION_UPPER_LIMIT);
    relay.open();
    assert(!setStepperPos(TEST_STEPPER_POSITION_LOWER_LIMIT));
    assert(!incrementStepperPos());
    assert(!decrementStepperPos());

    // fail when position of-of-bounds
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

static void test_wear_leveled_eeprom_object()
{
    int8_t value;
    uint16_t addr;

    /* test test macros */
    assert((TEST_EEPROM_START_ADDR +
            (TEST_EEPROM_QUEUE_ITEM_COUNT * sizeof(int8_t))) <
           EEPROM_SIZE_BYTES);
    assert(TEST_TRUNCATED_EEPROM_START_ADDR < EEPROM_SIZE_BYTES);
    assert((TEST_TRUNCATED_EEPROM_START_ADDR +
            (TEST_TRUNCATED_EEPROM_QUEUE_ITEM_COUNT * sizeof(int8_t))) >
           EEPROM_SIZE_BYTES);
    assert((TEST_TRUNCATED_EEPROM_START_ADDR +
            (TEST_TRUNCATED_EEPROM_ACTUAL_QUEUE_ITEM_COUNT * sizeof(int8_t))) <
           EEPROM_SIZE_BYTES);
    assert((TEST_TRUNCATED_EEPROM_START_ADDR +
            ((TEST_TRUNCATED_EEPROM_ACTUAL_QUEUE_ITEM_COUNT + 1) * sizeof(int8_t))) >
           EEPROM_SIZE_BYTES);

    WearLeveledEepromObject<int8_t> storage(TEST_EEPROM_START_ADDR,
                                            TEST_EEPROM_QUEUE_ITEM_COUNT);
    addr = storage.m_curAddr;

    assert(storage.m_startAddr == TEST_EEPROM_START_ADDR);
    assert(storage.m_endAddr == storage.m_startAddr +
                                (TEST_EEPROM_QUEUE_ITEM_COUNT * sizeof(int8_t)));
    assert(storage.m_circularQueueItemCount == TEST_EEPROM_QUEUE_ITEM_COUNT);
    assert(storage.m_circularQueueItemSize ==
           sizeof(WearLeveledEepromObject<int8_t>::CircularQueueItem));

    /* value retrieval */
    // will fail first time value gets stored
    assert(storage.get(value) == TEST_EEPROM_STORED_VALUE);

    /* value storage */
    bool queueRollover;
    bool regularStorageTested = false;
    bool rolloverStorageTested = false;
    for (int i = 0; i < TEST_EEPROM_QUEUE_ITEM_COUNT; i++) {
        addr = storage.m_curAddr;
        queueRollover = (storage.m_curAddr == storage.m_endAddr);
        storage.put(value);
        assert(storage.get(value) == TEST_EEPROM_STORED_VALUE);

        if (queueRollover) {
            assert(storage.m_curAddr == storage.m_startAddr);
            rolloverStorageTested = true;
        } else {
            assert(storage.m_curAddr == (addr + storage.m_circularQueueItemSize));
            regularStorageTested = true;
        }

        if (regularStorageTested && rolloverStorageTested) {
            break;
        }
    }

    /* circular queue truncation */
    WearLeveledEepromObject<int8_t> truncatedStorage(TEST_TRUNCATED_EEPROM_START_ADDR,
                                                     TEST_TRUNCATED_EEPROM_QUEUE_ITEM_COUNT);

    assert(truncatedStorage.m_circularQueueItemCount ==
           TEST_TRUNCATED_EEPROM_ACTUAL_QUEUE_ITEM_COUNT);
    assert(truncatedStorage.m_endAddr < EEPROM_SIZE_BYTES);
}

void test()
{
    test_smart_blinds();
    test_wear_leveled_eeprom_object();
}

