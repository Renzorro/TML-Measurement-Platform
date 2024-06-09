// TML_linear_actuator.h

// Setup
void linearActuatorMainSetup(void);
void linearActuatorAuxiliarySetup(void);
void linearActuatorSetup(void);

// Main Linear Actuator
void linearActuatorMainStop(void);
void linearActuatorMainUp(int speed = 100);
void linearActuatorMainDown(int speed = 100);

// Auxiliary Linear Actuator
void linearActuatorAuxiliaryStop(void);
void linearActuatorAuxiliaryUp(int speed = 100);
void linearActuatorAuxiliaryDown(int speed = 100);

// Home
void linearActuatorMainHome(void);
void linearActuatorAuxiliaryHome(void);
void linearActuatorHome(void);
