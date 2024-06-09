// TML_load_cell.cpp

#include "TML_load_cell.h"

#include "TML_eeprom.h"
#include "TML_imu.h"
#include "TML_desktop_communication.h"

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <HX711.h>

#define CALIBRATION_MASS 5 // calibration mass in kilograms
#define CALIBRATION_READINGS 10 // number of readings to average during calibration
#define AVERAGE_READINGS 1 // number of readings to average during operation

#define ADDRESS_CALIBRATION_FACTOR_M_FOREFOOT 0 // EEPROM address of main stage forefoot calibration factor
#define ADDRESS_CALIBRATION_FACTOR_M_BACKFOOT sizeof(double) // EEPROM address of main stage backfoot calibration factor
#define ADDRESS_CALIBRATION_FACTOR_A_FOREFOOT sizeof(double) * 2 // EEPROM address of auxiliary stage forefoot calibration factor
#define ADDRESS_CALIBRATION_FACTOR_A_BACKFOOT sizeof(double) * 3 // EEPROM address of auxiliary stage backfoot calibration factor

#define PIN_LOAD_CELL_DAT_M_FOREFOOT 15 // M_FOREFOOT_DAT_3V3
#define PIN_LOAD_CELL_CLK_M_FOREFOOT 16 // M_FOREFOOT_CLK_3V3
#define PIN_LOAD_CELL_DAT_M_BACKFOOT 17 // M_BACKFOOT_DAT_3V3
#define PIN_LOAD_CELL_CLK_M_BACKFOOT 18 // M_BACKFOOT_CLK_3V3

#define PIN_LOAD_CELL_DAT_A_FOREFOOT 40 // A_FOREFOOT_DAT_3V3
#define PIN_LOAD_CELL_CLK_A_FOREFOOT 39 // A_FOREFOOT_CLK_3V3
#define PIN_LOAD_CELL_DAT_A_BACKFOOT 38 // A_BACKFOOT_DAT_3V3
#define PIN_LOAD_CELL_CLK_A_BACKFOOT 37 // A_BACKFOOT_CLK_3V3

HX711 load_cell_m_forefoot;
HX711 load_cell_m_backfoot;
HX711 load_cell_a_forefoot;
HX711 load_cell_a_backfoot;

double calibration_factor_m_forefoot = 0;
double calibration_factor_m_backfoot = 0;
double calibration_factor_a_forefoot = 0;
double calibration_factor_a_backfoot = 0;

double reading_m_forefoot = 0;
double reading_m_backfoot = 0;
double reading_a_forefoot = 0;
double reading_a_backfoot = 0;

void loadCellMainForefootSetScale(void){
  if (calibration_factor_m_forefoot != 0){
    load_cell_m_forefoot.set_scale(calibration_factor_m_forefoot);
    load_cell_m_forefoot.tare();
  }
  else{
    Serial.println("Error: main stage forefoot calibration factor not set!");
  }

  return;
}

void loadCellMainBackfootSetScale(void){
  if (calibration_factor_m_backfoot != 0){
    load_cell_m_backfoot.set_scale(calibration_factor_m_backfoot);
    load_cell_m_backfoot.tare();
  }
  else{
    Serial.println("Error: main stage backfoot calibration factor not set!");
  }

  return;
}

void loadCellAuxiliaryForefootSetScale(void){
  if (calibration_factor_a_forefoot != 0){
    load_cell_a_forefoot.set_scale(calibration_factor_a_forefoot);
    load_cell_a_forefoot.tare();
  }
  else{
    Serial.println("Error: auxiliary stage forefoot calibration factor not set!");
  }

  return;
}

void loadCellAuxiliaryBackfootSetScale(void){
  if (calibration_factor_a_backfoot != 0){
    load_cell_a_backfoot.set_scale(calibration_factor_a_backfoot);
    load_cell_a_backfoot.tare();
  }
  else{
    Serial.println("Error: auxiliary stage backfoot calibration factor not set!");
  }
  
  return;
}

void loadCellSetScale(void){
  loadCellMainForefootSetScale();
  loadCellMainBackfootSetScale();
  loadCellAuxiliaryForefootSetScale();
  loadCellAuxiliaryBackfootSetScale();

  return;
}

void loadCellMainForefootSetup(void){
  load_cell_m_forefoot.begin(PIN_LOAD_CELL_DAT_M_FOREFOOT, PIN_LOAD_CELL_CLK_M_FOREFOOT);

  // delay(1000);

  // if (!load_cell_m_forefoot.is_ready()){
  //   Serial.println("Error: main stage forefoot load cells failed to initialize!");

  //   return;
  // }

  eepromGet(ADDRESS_CALIBRATION_FACTOR_M_FOREFOOT, calibration_factor_m_forefoot);
  Serial.print("Main stage forefoot calibration factor: ");
  Serial.println(calibration_factor_m_forefoot);

  loadCellMainForefootSetScale();
  Serial.println("Main stage forefoot load cells initialized!");

  return;
}

void loadCellMainBackfootSetup(void){
  load_cell_m_backfoot.begin(PIN_LOAD_CELL_DAT_M_BACKFOOT, PIN_LOAD_CELL_CLK_M_BACKFOOT);

  // delay(1000);

  // if (!load_cell_m_backfoot.is_ready()){
  //   Serial.println("Error: main stage backfoot load cells failed to initialize!");

  //   return;
  // }

  eepromGet(ADDRESS_CALIBRATION_FACTOR_M_BACKFOOT, calibration_factor_m_backfoot);
  Serial.print("Main stage backfoot calibration factor: ");
  Serial.println(calibration_factor_m_backfoot);

  loadCellMainBackfootSetScale();
  Serial.println("Main stage backfoot load cells initialized!");

  return;
}

void loadCellAuxiliaryForefootSetup(void){
  load_cell_a_forefoot.begin(PIN_LOAD_CELL_DAT_A_FOREFOOT, PIN_LOAD_CELL_CLK_A_FOREFOOT);

  // delay(1000);

  // if (!load_cell_a_forefoot.is_ready()){
  //   Serial.println("Error: auxiliary stage forefoot load cells failed to initialize!");

  //   return;
  // }

  eepromGet(ADDRESS_CALIBRATION_FACTOR_A_FOREFOOT, calibration_factor_a_forefoot);
  Serial.print("Auxiliary stage forefoot calibration factor: ");
  Serial.println(calibration_factor_a_forefoot);

  loadCellAuxiliaryForefootSetScale();
  Serial.println("Auxiliary stage forefoot load cells initialized!");

  return;
}

void loadCellAuxiliaryBackfootSetup(void){
  load_cell_a_backfoot.begin(PIN_LOAD_CELL_DAT_A_BACKFOOT, PIN_LOAD_CELL_CLK_A_BACKFOOT);

  // delay(1000);

  // if (!load_cell_a_backfoot.is_ready()){
  //   Serial.println("Error: auxiliary stage backfoot load cells failed to initialize!");

  //   return;
  // }

  eepromGet(ADDRESS_CALIBRATION_FACTOR_A_BACKFOOT, calibration_factor_a_backfoot);
  Serial.print("Auxiliary stage backfoot calibration factor: ");
  Serial.println(calibration_factor_a_backfoot);

  loadCellAuxiliaryBackfootSetScale();
  Serial.println("Auxiliary stage backfoot load cells initialized!");

  return;
}

void loadCellSetup(void){
  loadCellMainForefootSetup();
  loadCellMainBackfootSetup();
  loadCellAuxiliaryForefootSetup();
  loadCellAuxiliaryBackfootSetup();

  return;
}

void loadCellMainForefootCalibration(void){
  if (!load_cell_m_forefoot.is_ready()){
    Serial.println("Error: main stage forefoot load cells not ready!");

    return;
  }

  Serial.println("Calibrating main stage forefoot load cells...");

  Serial.println("Tare... remove any weights from the main stage forefoot load cells (10 seconds).");
  delay(10000);
  load_cell_m_forefoot.tare();
  Serial.println("Tare completed for main stage forefoot load cells.");

  Serial.println("Calibrate... place 5 kg on main stage forefoot load cells (10 seconds).");
  delay(10000);
  calibration_factor_m_forefoot = load_cell_m_forefoot.get_value(CALIBRATION_READINGS) / CALIBRATION_MASS;
  Serial.println("calibration completed; remove any weights from the main stage forefoot load cells (10 seconds).");
  delay(10000);

  eepromPut(ADDRESS_CALIBRATION_FACTOR_M_FOREFOOT, calibration_factor_m_forefoot);
  eepromCommit();
  
  loadCellMainForefootSetScale();
  Serial.println("... main stage forefoot load cells calibrated!");
  
  return;
}

void loadCellMainBackfootCalibration(void){
  if (!load_cell_m_backfoot.is_ready()){
    Serial.println("Error: main stage backfoot load cells not ready!");

    return;
  }

  Serial.println("Calibrating main stage backfoot load cells...");

  Serial.println("Tare... remove any weights from the main stage backfoot load cells (10 seconds).");
  delay(10000);
  load_cell_m_backfoot.tare();
  Serial.println("Tare completed for main stage backfoot load cells.");

  Serial.println("Calibrate... place 5 kg on main stage backfoot load cells (10 seconds).");
  delay(10000);
  calibration_factor_m_backfoot = load_cell_m_backfoot.get_value(CALIBRATION_READINGS) / (CALIBRATION_MASS * cos(imuMainAngleRadians()));
  Serial.println("... calibration completed; remove any weights from the main stage backfoot load cells (10 seconds).");
  delay(10000);

  eepromPut(ADDRESS_CALIBRATION_FACTOR_M_BACKFOOT, calibration_factor_m_backfoot);
  eepromCommit();
  
  loadCellMainBackfootSetScale();
  Serial.println("Main stage backfoot load cells calibrated!");

  return;
}

void loadCellAuxiliaryForefootCalibration(void){
  if (!load_cell_a_forefoot.is_ready()){
    Serial.println("Error: auxiliary stage forefoot load cells not ready!");

    return;
  }

  Serial.println("Calibrating auxiliary stage forefoot load cells...");

  Serial.println("Tare... remove any weights from the auxiliary stage forefoot load cells (10 seconds).");
  delay(10000);
  load_cell_a_forefoot.tare();
  Serial.println("Tare completed for auxiliary stage forefoot load cells.");

  Serial.println("Calibrate... place 5 kg on auxiliary stage forefoot load cells (10 seconds).");
  delay(10000);
  calibration_factor_a_forefoot = load_cell_a_forefoot.get_value(CALIBRATION_READINGS) / CALIBRATION_MASS;
  Serial.println("... calibration completed; remove any weights from the auxiliary stage forefoot load cells (10 seconds).");
  delay(10000);
  
  eepromPut(ADDRESS_CALIBRATION_FACTOR_A_FOREFOOT, calibration_factor_a_forefoot);
  eepromCommit();
  
  loadCellAuxiliaryForefootSetScale();
  Serial.println("Auxiliary stage forefoot load cells calibrated!");

  return;
}

void loadCellAuxiliaryBackfootCalibration(void){
  if (!load_cell_a_backfoot.is_ready()){
    Serial.println("Error: auxiliary stage backfoot load cells not ready!");

    return;
  }

  Serial.println("Calibrating auxiliary stage backfoot load cells...");

  Serial.println("Tare... remove any weights from the auxiliary stage backfoot load cells (10 seconds).");
  delay(10000);
  load_cell_a_backfoot.tare();
  Serial.println("Tare completed for auxiliary stage backfoot load cells.");

  Serial.println("Calibrate... place 5 kg on auxiliary stage backfoot load cells (10 seconds).");
  delay(10000);
  calibration_factor_a_backfoot = load_cell_a_backfoot.get_value(CALIBRATION_READINGS) / (CALIBRATION_MASS * cos(imuAuxiliaryAngleRadians()));
  Serial.println("... calibration completed; remove any weights from the auxiliary stage backfoot load cells (10 seconds).");
  delay(10000);

  eepromPut(ADDRESS_CALIBRATION_FACTOR_A_BACKFOOT, calibration_factor_a_backfoot);
  eepromCommit();
  
  loadCellAuxiliaryBackfootSetScale();
  Serial.println("Auxiliary stage backfoot load cells calibrated!");

  return;
}

void loadCellCalibration(void){
  loadCellMainForefootCalibration();
  loadCellMainBackfootCalibration();
  loadCellAuxiliaryForefootCalibration();
  loadCellAuxiliaryBackfootCalibration();

  return;
}

void loadCellMainForefootTare(void){
  load_cell_m_forefoot.tare();

  return;
}

void loadCellMainBackfootTare(void){
  load_cell_m_backfoot.tare();

  return;
}

void loadCellAuxiliaryForefootTare(void){
  load_cell_a_forefoot.tare();

  return;
}

void loadCellAuxiliaryBackfootTare(void){
  load_cell_a_backfoot.tare();

  return;
}

void loadCellTare(void){
  loadCellMainForefootTare();
  loadCellMainBackfootTare();
  loadCellAuxiliaryForefootTare();
  loadCellAuxiliaryBackfootTare();
  
  return;
}

double loadCellMainForefootMassNormal(void){
  reading_m_forefoot = load_cell_m_forefoot.get_units(AVERAGE_READINGS);

  return reading_m_forefoot;
}

double loadCellMainForefootMassGravity(void){
  reading_m_forefoot = load_cell_m_forefoot.get_units(AVERAGE_READINGS);

  return reading_m_forefoot;
}

double loadCellMainBackfootMassNormal(void){
  reading_m_backfoot = load_cell_m_backfoot.get_units(AVERAGE_READINGS);

  return reading_m_backfoot;
}

double loadCellMainBackfootMassGravity(void){
  reading_m_backfoot = load_cell_m_backfoot.get_units(AVERAGE_READINGS) / cos(imuMainAngleRadians());

  return reading_m_backfoot;
}

double loadCellAuxiliaryForefootMassNormal(void){
  reading_a_forefoot = load_cell_a_forefoot.get_units(AVERAGE_READINGS);

  return reading_a_forefoot;
}

double loadCellAuxiliaryForefootMassGravity(void){
  reading_a_forefoot = load_cell_a_forefoot.get_units(AVERAGE_READINGS);

  return reading_a_forefoot;
}

double loadCellAuxiliaryBackfootMassNormal(void){
  reading_a_backfoot = load_cell_a_backfoot.get_units(AVERAGE_READINGS);

  return reading_a_backfoot;
}

double loadCellAuxiliaryBackfootMassGravity(void){
  reading_a_backfoot = load_cell_a_backfoot.get_units(AVERAGE_READINGS) / cos(imuAuxiliaryAngleRadians());

  return reading_a_backfoot;
}

double loadCellMainForefootForceNormal(void){
  reading_m_forefoot = load_cell_m_forefoot.get_units(AVERAGE_READINGS) * 9.81;

  return reading_m_forefoot;
}

double loadCellMainForefootForceGravity(void){
  reading_m_forefoot = load_cell_m_forefoot.get_units(AVERAGE_READINGS) * 9.81;

  return reading_m_forefoot;
}

double loadCellMainBackfootForceNormal(void){
  reading_m_backfoot = load_cell_m_backfoot.get_units(AVERAGE_READINGS) * 9.81;

  return reading_m_backfoot;
}

double loadCellMainBackfootForceGravity(void){
  reading_m_backfoot = load_cell_m_backfoot.get_units(AVERAGE_READINGS) / cos(imuMainAngleRadians()) * 9.81;

  return reading_m_backfoot;
}

double loadCellAuxiliaryForefootForceNormal(void){
  reading_a_forefoot = load_cell_a_forefoot.get_units(AVERAGE_READINGS) * 9.81;

  return reading_a_forefoot;
}

double loadCellAuxiliaryForefootForceGravity(void){
  reading_a_forefoot = load_cell_a_forefoot.get_units(AVERAGE_READINGS) * 9.81;

  return reading_a_forefoot;
}

double loadCellAuxiliaryBackfootForceNormal(void){
  reading_a_backfoot = load_cell_a_backfoot.get_units(AVERAGE_READINGS) * 9.81;

  return reading_a_backfoot;
}

double loadCellAuxiliaryBackfootForceGravity(void){
  reading_a_backfoot = load_cell_a_backfoot.get_units(AVERAGE_READINGS) / cos(imuAuxiliaryAngleRadians()) * 9.81;

  return reading_a_backfoot;
}
