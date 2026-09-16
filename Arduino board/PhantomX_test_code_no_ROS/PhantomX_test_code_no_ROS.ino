#include "ax12.h"

// Servo IDs
#define BASE_SERVO      1
#define SHOULDER_SERVO  2
#define ELBOW_SERVO     3
#define WRIST_SERVO     4
#define GRIPPER_SERVO   5

// ------------------------------------------------------------
// Servo positions
// ------------------------------------------------------------

// Base stays centered
#define BASE_POSITION 512

// L-shaped position
#define L_SHOULDER 205
#define L_ELBOW    205
#define L_WRIST    512

// Straight vertical position
#define V_SHOULDER 205
#define V_ELBOW    512
#define V_WRIST    512

// Gripper
#define GRIPPER_POSITION 0

// Movement speed
#define ARM_SPEED 50

// Time to stay in each position
#define POSITION_DELAY 5000
#define GRIPPER_DELAY 3000


void setServoSpeed(int id, int speed)
{
  ax12SetRegister2(id, AX_GOAL_SPEED_L, speed);
}


void setSpeeds()
{
  setServoSpeed(BASE_SERVO, ARM_SPEED);
  setServoSpeed(SHOULDER_SERVO, ARM_SPEED);
  setServoSpeed(ELBOW_SERVO, ARM_SPEED);
  setServoSpeed(WRIST_SERVO, ARM_SPEED);
  setServoSpeed(GRIPPER_SERVO, ARM_SPEED);
}


void LPosition()
{
  SetPosition(BASE_SERVO, BASE_POSITION);

  SetPosition(SHOULDER_SERVO, L_SHOULDER);
  SetPosition(ELBOW_SERVO, L_ELBOW);
  SetPosition(WRIST_SERVO, L_WRIST);

  SetPosition(GRIPPER_SERVO, GRIPPER_POSITION);
}


void verticalPosition()
{
  SetPosition(BASE_SERVO, BASE_POSITION);

  SetPosition(SHOULDER_SERVO, V_SHOULDER);
  SetPosition(ELBOW_SERVO, V_ELBOW);
  SetPosition(WRIST_SERVO, V_WRIST);

  SetPosition(GRIPPER_SERVO, GRIPPER_POSITION);
}


void setup()
{
  setSpeeds();

  // Start in the L position
  LPosition();
  SetPosition(GRIPPER_SERVO, 0);

  delay(POSITION_DELAY);
}


void loop()
{
  // L -> vertical
  verticalPosition();

  delay(POSITION_DELAY);

  SetPosition(GRIPPER_SERVO, 512);
  delay(GRIPPER_DELAY);
  SetPosition(GRIPPER_SERVO, 0);
  delay(GRIPPER_DELAY);
  SetPosition(GRIPPER_SERVO, 512);
  delay(GRIPPER_DELAY);
  SetPosition(GRIPPER_SERVO, 0);
  delay(GRIPPER_DELAY);

  // Vertical -> L
  LPosition();

  delay(POSITION_DELAY);
}
