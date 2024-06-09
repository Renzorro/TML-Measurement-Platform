// TML_eeprom.cpp

#include "TML_eeprom.h"

#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_SIZE 512

void eepromSetup(void){
  if (!EEPROM.begin(EEPROM_SIZE)){
    Serial.println("Error: EEPROM failed to initialize!");

    return;
  }
  
  Serial.println("EEPROM initialized!");

  return;
}

void eepromGet(int address, double &value){
  EEPROM.get(address, value);

  return;
}

void eepromPut(int address, double &value){
  EEPROM.put(address, value);

  return;
}

void eepromCommit(void){
  EEPROM.commit();

  return;
}
