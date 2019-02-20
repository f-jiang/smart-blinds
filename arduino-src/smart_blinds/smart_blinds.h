#ifndef SMART_BLINDS
#define SMART_BLINDS

#include <limits.h>
#include <stdint.h>

#include <Stepper.h>
#include <Relay.h>
#include <AceButton.h>

#define STEP_INCREMENT_SIZE                     1
#define STEPPER_POSITION_DEFAULT                0
#define STEPPER_POSITION_UPPER_LIMIT_DEFAULT    INT16_MAX
#define STEPPER_POSITION_LOWER_LIMIT_DEFAULT    INT16_MIN

typedef int16_t stepper_pos_t;

extern Stepper stepper;
extern Relay relay;
extern ace_button::AceButton btn;

void handleBtnEvent(ace_button::AceButton*, uint8_t, uint8_t);
bool isPosInverted();
bool isStepperCalibrated();
bool isPosOutOfBounds(stepper_pos_t pos);
bool getStepperPos(stepper_pos_t& pos);
bool setStepperPos(stepper_pos_t pos);
bool incrementStepperPos();
bool decrementStepperPos();
bool getStepperPosLowerLimit(stepper_pos_t& pos);
bool setStepperPosLowerLimit(stepper_pos_t pos);
bool getStepperPosUpperLimit(stepper_pos_t& pos);
bool setStepperPosUpperLimit(stepper_pos_t pos);

#endif  // SMART_BLINDS

