#ifndef SMART_BLINDS
#define SMART_BLINDS

#include <limits.h>

#include <Stepper.h>
#include <Relay.h>

#define STEP_INCREMENT_SIZE                     1
#define STEPPER_POSITION_DEFAULT                0
#define STEPPER_POSITION_UPPER_LIMIT_DEFAULT    INT_MAX
#define STEPPER_POSITION_LOWER_LIMIT_DEFAULT    INT_MIN

enum StepperPositionScaleType {
    NORMAL,
    INVERTED
};

extern Stepper stepper;
extern Relay relay;
extern int stepperPos;
extern int stepperPosLowerLimit;
extern int stepperPosUpperLimit;

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

#endif  // SMART_BLINDS

