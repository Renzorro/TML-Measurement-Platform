// TML_eeprom.h

// Setup
void eepromSetup(void);

// Get
void eepromGet(int address, double &value);

// Put
void eepromPut(int address, double &value);

// Commit
void eepromCommit(void);
