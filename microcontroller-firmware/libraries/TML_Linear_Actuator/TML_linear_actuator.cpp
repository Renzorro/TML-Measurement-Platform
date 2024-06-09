// TML_linear_actuator.cpp

#include "TML_linear_actuator.h"

#include "TML_imu.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_LSM6DS3TRC.h>

#define HOME_ANGLE 1 // home angle in degrees

#define PIN_LINEAR_ACTUATOR_M_SIG_POS 4 // M_LINEAR_ACTUATOR_SIG+
#define PIN_LINEAR_ACTUATOR_M_SIG_NEG 5 // M_LINEAR_ACTUATOR_SIG-

#define PIN_LINEAR_ACTUATOR_A_SIG_POS 1 // A_LINEAR_ACTUATOR_SIG+
#define PIN_LINEAR_ACTUATOR_A_SIG_NEG 2 // A_LINEAR_ACTUATOR_SIG-

int pwm_channel_m_sig_pos = 0; // PWM channel for M_LINEAR_ACTUATOR_SIG+
int pwm_channel_m_sig_neg = 1; // PWM channel for M_LINEAR_ACTUATOR_SIG-
int pwm_channel_a_sig_pos = 2; // PWM channel for A_LINEAR_ACTUATOR_SIG+
int pwm_channel_a_sig_neg = 3; // PWM channel for A_LINEAR_ACTUATOR_SIG-
int frequency = 40000; // PWM frequency of 40 kHz
int resolution = 8; // 8-bit resolution (256 possible values) for duty cycle

void linearActuatorMainSetup(void){
  ledcSetup(pwm_channel_m_sig_pos, frequency, resolution);
  ledcSetup(pwm_channel_m_sig_neg, frequency, resolution);

  ledcAttachPin(PIN_LINEAR_ACTUATOR_M_SIG_POS, pwm_channel_m_sig_pos);
  ledcAttachPin(PIN_LINEAR_ACTUATOR_M_SIG_NEG, pwm_channel_m_sig_neg);

  ledcWrite(pwm_channel_m_sig_pos, 0);
  ledcWrite(pwm_channel_m_sig_neg, 0);

  return;
}

void linearActuatorAuxiliarySetup(void){
  ledcSetup(pwm_channel_a_sig_pos, frequency, resolution);
  ledcSetup(pwm_channel_a_sig_neg, frequency, resolution);

  ledcAttachPin(PIN_LINEAR_ACTUATOR_A_SIG_POS, pwm_channel_a_sig_pos);
  ledcAttachPin(PIN_LINEAR_ACTUATOR_A_SIG_NEG, pwm_channel_a_sig_neg);

  ledcWrite(pwm_channel_a_sig_pos, 0);
  ledcWrite(pwm_channel_a_sig_neg, 0);

  return;
}

void linearActuatorSetup(void){
  linearActuatorMainSetup();
  linearActuatorAuxiliarySetup();

  return;
}

void linearActuatorMainStop(void){
  ledcWrite(pwm_channel_m_sig_pos, 0);
  ledcWrite(pwm_channel_m_sig_neg, 0);

  return;
}

void linearActuatorMainUp(int speed){
  ledcWrite(pwm_channel_m_sig_pos, 0);
  ledcWrite(pwm_channel_m_sig_neg, speed * 255 / 100);

  return;
}

void linearActuatorMainDown(int speed){
  ledcWrite(pwm_channel_m_sig_pos, speed * 255 / 100);
  ledcWrite(pwm_channel_m_sig_neg, 0);

  return;
}

void linearActuatorAuxiliaryStop(void){
  ledcWrite(pwm_channel_a_sig_pos, 0);
  ledcWrite(pwm_channel_a_sig_neg, 0);

  return;
}

void linearActuatorAuxiliaryUp(int speed){
  ledcWrite(pwm_channel_a_sig_pos, 0);
  ledcWrite(pwm_channel_a_sig_neg, speed * 255 / 100);

  return;
}

void linearActuatorAuxiliaryDown(int speed){
  ledcWrite(pwm_channel_a_sig_pos, speed * 255 / 100);
  ledcWrite(pwm_channel_a_sig_neg, 0);

  return;
}

void linearActuatorMainHome(void){
  while (imuMainAngleDegrees() > HOME_ANGLE){
    linearActuatorMainDown();
  }

  linearActuatorMainStop();
  Serial.println("Main stage linear actuator homed!");

  return;
}

void linearActuatorAuxiliaryHome(void){
  while (imuAuxiliaryAngleDegrees() > HOME_ANGLE){
    linearActuatorAuxiliaryDown();
  }

  linearActuatorAuxiliaryStop();
  Serial.println("Auxiliary stage linear actuator homed!");

  return;
}

void linearActuatorHome(void){
  linearActuatorMainHome();
  linearActuatorAuxiliaryHome();

  return;
}
