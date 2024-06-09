/*
Board: ESP32-S3-DEVKITC-1-N8R8
Board Module: ESP32S3 Dev Module
Description: firmware code for main stage of autonomous equinus wedge
*/

// Imports
#include "TML_desktop_communication.h" // Custom library for desktop GUI communication
#include "TML_remote_communication.h" // Custom library for remote communication
#include "TML_control_algorithms.h" // Custom library for device control algorithms
#include "TML_queue.h" // Custom library for implementing queues
#include "TML_lock.h" // Custom library for implementing locks
#include "TML_eeprom.h" // Custom library for implementing EEPROM
#include "TML_imu.h" // Custom library for implementing IMU
#include "TML_linear_actuator.h" // Custom library for implementing linear actuator
#include "TML_load_cell.h" // Custom library for implementing load cells
#include "TML_capacitive_screen.h" // Custom library for implementing capacitive touch screen

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_now.h>
#include <WiFi.h>
#include <esp_random.h>

#define MAIN_MAC {0xDC, 0xDA, 0x0C, 0x4A, 0xF8, 0xA0}
#define REMOTE_MAC {0x0C, 0x8B, 0x95, 0x96, 0x42, 0x4c}

#define RC_SWITCHR 0
#define RC_SWITCHM 1
#define RC_SWITCHL  3
#define RC_RU    4
#define RC_RD    5
#define RC_RS    6
#define RC_LU    7
#define RC_LD    8
#define RC_LS    9


typedef struct remote_update {
  unsigned int msg_type;
  int main_fore;
  int main_heel;
  int main_ang;
  int aux_fore;
  int aux_heel;
  int aux_ang;
  bool main_green;
  bool aux_green;
} remote_update;

typedef struct remote_lock {
  unsigned int msg_type;
  char str[32];
} remote_lock;

typedef struct remote_command {
  unsigned int switchR = 0;
  unsigned int switchM = 0;
  unsigned int switchL = 0;
  unsigned int manR = 0;
  unsigned int manL = 0;
} remote_command;

void OnESPSent(const uint8_t *mac, esp_now_send_status_t status);
void OnESPRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

void r2m(unsigned int command);

class remoteIO {

  private:
    uint8_t target_mac[6] = REMOTE_MAC;
    esp_now_peer_info_t peerInfo;

    void sendCheck(esp_err_t result) {
      if (result == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }
    }

  public:
    remote_update msg_out;
    remote_command msg_in;
    remote_lock rem_lock;

    remoteIO() {}

    void begin() {
      // Set device as a Wi-Fi Station
      WiFi.mode(WIFI_STA);
      
      // Init ESP-NOW
      if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
      }
      
      // Register functions for send & recieve callback
      esp_now_register_send_cb(OnESPSent);
      esp_now_register_recv_cb(OnESPRecv);

      // Register peer
      memcpy(peerInfo.peer_addr, target_mac, 6);
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;

      // Add peer        
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
      }
    }

    void toRemote(int main_fore, int main_heel, int main_ang, int aux_fore, int aux_heel, int aux_ang, bool main_green, bool aux_green) {
      msg_out.msg_type = 0;
      msg_out.main_fore = main_fore;
      msg_out.main_heel = main_heel;
      msg_out.main_ang = main_ang;
      msg_out.aux_fore = aux_fore;
      msg_out.aux_heel = aux_heel;
      msg_out.aux_ang = aux_ang;
      msg_out.main_green = main_green;
      msg_out.aux_green = aux_green;
      
      // Send message via ESP-NOW
      esp_err_t result = esp_now_send(target_mac, (uint8_t *) &msg_out, sizeof(msg_out));
      // sendCheck(result);
    }

    void lockRemote(char str[]) {
      rem_lock.msg_type = 1;
      memcpy(&rem_lock.str, str, sizeof(rem_lock.str));
      esp_err_t result = esp_now_send(target_mac, (uint8_t *) &rem_lock, sizeof(rem_lock));
      sendCheck(result);
    }

    void unlockRemote() {
      unsigned int msg_type = 2;
      esp_err_t result = esp_now_send(target_mac, (uint8_t *) &msg_type, sizeof(msg_type));
      sendCheck(result);
    }

    void fromRemote() {

      if (msg_in.switchR) {
        r2m(RC_SWITCHR);
        Serial.println("R Button");
      }

      if (msg_in.switchM) {
        r2m(RC_SWITCHM);
        Serial.println("M Button");
      }

      if (msg_in.switchL) {
        r2m(RC_SWITCHL);
        Serial.println("L Button");
      }

      switch (msg_in.manR) {
        case 1:
          r2m(RC_RU);
          Serial.println("Right Up");
          break;
        
        case 2:
          r2m(RC_RD);
          Serial.println("Right Down");
          break;

        case 3:
          r2m(RC_RS);
          Serial.println("Right Stop");
          break;
      }

      switch (msg_in.manL) {
        case 1:
          r2m(RC_LU);
          Serial.println("Left Up");
          break;
        
        case 2:
          r2m(RC_LD);
          Serial.println("Left Down");
          break;

        case 3:
          r2m(RC_LS);
          Serial.println("Left Stop");
          break;
      }
    }
};

remoteIO rem;

void OnESPSent(const uint8_t *mac, esp_now_send_status_t status) {
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnESPRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rem.msg_in, incomingData, sizeof(rem.msg_in));
  rem.fromRemote();
}

// Object Declarations
Queue cbuf; // Centralized
Queue rbuf; // Remote
Queue sbuf; // Capacitive Screen

Lock lock;

// Function declarations
int command_to_foot(char command);
int command_to_controller(char command);
void both_tare(void);
void both_home(void);
void both_auto(void);
void both_calibrate(void);
void main_up(void);
void main_stop(void);
void main_down(void);
void main_home(void);
void main_auto(void);
void main_tare(void);
void aux_up(void);
void aux_stop(void);
void aux_down(void);
void aux_home(void);
void aux_auto(void);
void aux_tare(void);

char input = '\0';
char input_r = '\0';
char input_s = '\0';
char action_s = '\0';
char next_command;

void setup() {
  Serial.begin(115200);
  //while(!Serial);

  serial_send_update("Flash setup beginning..."); // For debugging

  initQueue(&cbuf);
  initQueue(&rbuf);

  eepromSetup();

  imuSetup();

  loadCellSetup();

  linearActuatorSetup();

  displaySetup();

  rem.begin(); // MUST HAVE THIS IN SETUP
}

void loop() {
  // Send info to serial
  if(Serial) serial_send_stream(imuMainAngleDegrees(),loadCellMainBackfootForceNormal(),loadCellMainForefootForceNormal(),imuAuxiliaryAngleDegrees(),loadCellAuxiliaryBackfootForceNormal(),loadCellAuxiliaryForefootForceNormal());
  //rem.toRemote((int)loadCellMainForefootForceNormal(), (int)loadCellMainBackfootForceNormal(), (int)imuMainAngleDegrees(), (int)loadCellAuxiliaryForefootForceNormal(), (int)loadCellAuxiliaryBackfootForceNormal(), (int)imuAuxiliaryAngleDegrees(), false, false);
  // DEBUG
  rem.toRemote((int)loadCellAuxiliaryForefootForceNormal(), (int)loadCellAuxiliaryBackfootForceNormal(), (int)imuAuxiliaryAngleDegrees(), (int)loadCellMainForefootForceNormal(), (int)loadCellMainBackfootForceNormal(), (int)imuMainAngleDegrees(), false, false);
  displayUpdate((int)loadCellMainForefootForceNormal(), (int)loadCellMainBackfootForceNormal(), (int)imuMainAngleDegrees(), (int)loadCellAuxiliaryForefootForceNormal(), (int)loadCellAuxiliaryBackfootForceNormal(), (int)imuAuxiliaryAngleDegrees());

  // Get commands from capacitive touch screen
  action_s = displayTouchChecker();
  if(action_s != '\0')
  {
    enqueue(&sbuf, action_s);
  }
  
  // Get inputs from desktop and rbuf and throw in the cbuf
  input_r = dequeue(&rbuf);
  if(input_r != '\0') // && try_acquire(&lock, command_to_foot(input_r), command_to_controller(input_r))
  {
    enqueue(&cbuf, input_r);
  }
  input_s = dequeue(&sbuf);
  if(input_s != '\0') //  && try_acquire(&lock, command_to_foot(input_s), command_to_controller(input_s))
  {
    enqueue(&cbuf, input_s);
  }
  input = getSerialInput();
  if(input != '\0') // && try_acquire(&lock, command_to_foot(input), command_to_controller(input))
  {
    enqueue(&cbuf, input);
  }

  next_command = dequeue(&cbuf);
  switch(next_command){
    case D2M_BOTH_TARE:
      both_tare();
      break;
    case D2M_MAIN_TARE:
      main_tare();
      break;
    case D2M_AUX_TARE:
      aux_tare();
      break;
    case D2M_AUX_STOP:
      aux_stop();
      break;
    case D2M_AUX_HOME:
      aux_home();
      break;
    case D2M_AUX_AUTO:
      home_state_update(AUX_HOME, 1.0, true);
      auto_start(AUX_AUTO);
      linearActuatorAuxiliaryUp(100);
      serial_send_update(M2D_AUX_AUTO);
      break;
    case D2M_MAIN_UP:
      main_up();
      break;
    case D2M_MAIN_DOWN:
      main_down(); 
      break; 
    case D2M_MAIN_STOP:
      main_stop();
      break;
    case D2M_MAIN_HOME:
      main_home(); 
      break; 
    case D2M_MAIN_AUTO:
      main_auto();
      break;
    case D2M_AUX_UP:
      aux_up();   
      break;
    case D2M_AUX_DOWN:
      aux_down();     
      break;
    case D2M_BOTH_CALIBRATE:
      auto_state_update(MAIN_AUTO, 1.0, true);
      auto_state_update(AUX_AUTO, 1.0, true);
      home_state_update(MAIN_HOME, 1.0, true);
      home_state_update(AUX_HOME, 1.0, true);
      linearActuatorMainStop();
      linearActuatorAuxiliaryStop();
      rem.lockRemote("LOCK");  
      serial_send_update(M2D_DISABLE_DESKTOP);
      serial_send_update(M2D_BOTH_CALIBRATE);
      loadCellCalibration();
      serial_send_update(M2D_BOTH_CALIBRATE_DONE);
      serial_send_update(M2D_ENABLE_DESKTOP);
      rem.unlockRemote();
    case D2M_MAIN_HEEL_CALIBRATE:
      auto_state_update(MAIN_AUTO, 1.0, true);
      auto_state_update(AUX_AUTO, 1.0, true);
      home_state_update(MAIN_HOME, 1.0, true);
      home_state_update(AUX_HOME, 1.0, true);
      linearActuatorMainStop();
      linearActuatorAuxiliaryStop();
      rem.lockRemote("LOCK");
      serial_send_update(M2D_DISABLE_DESKTOP);
      serial_send_update(M2D_BOTH_CALIBRATE);
      loadCellMainBackfootCalibration();
      serial_send_update(M2D_BOTH_CALIBRATE_DONE);
      serial_send_update(M2D_ENABLE_DESKTOP);
      rem.unlockRemote();
    case D2M_MAIN_FORE_CALIBRATE:
      auto_state_update(MAIN_AUTO, 1.0, true);
      auto_state_update(AUX_AUTO, 1.0, true);
      home_state_update(MAIN_HOME, 1.0, true);
      home_state_update(AUX_HOME, 1.0, true);
      linearActuatorMainStop();
      linearActuatorAuxiliaryStop();
      rem.lockRemote("LOCK");
      serial_send_update(M2D_DISABLE_DESKTOP);
      serial_send_update(M2D_BOTH_CALIBRATE);
      loadCellMainForefootCalibration();
      serial_send_update(M2D_BOTH_CALIBRATE_DONE);
      serial_send_update(M2D_ENABLE_DESKTOP);
      rem.unlockRemote();
    case D2M_AUX_HEEL_CALIBRATE:
      auto_state_update(MAIN_AUTO, 1.0, true);
      auto_state_update(AUX_AUTO, 1.0, true);
      home_state_update(MAIN_HOME, 1.0, true);
      home_state_update(AUX_HOME, 1.0, true);
      linearActuatorMainStop();
      linearActuatorAuxiliaryStop();
      rem.lockRemote("LOCK");
      serial_send_update(M2D_DISABLE_DESKTOP);
      serial_send_update(M2D_BOTH_CALIBRATE);
      loadCellAuxiliaryBackfootCalibration();
      serial_send_update(M2D_BOTH_CALIBRATE_DONE);
      serial_send_update(M2D_ENABLE_DESKTOP);
      rem.unlockRemote();
    case D2M_AUX_FORE_CALIBRATE:
      auto_state_update(MAIN_AUTO, 1.0, true);
      auto_state_update(AUX_AUTO, 1.0, true);
      home_state_update(MAIN_HOME, 1.0, true);
      home_state_update(AUX_HOME, 1.0, true);
      linearActuatorMainStop();
      linearActuatorAuxiliaryStop();
      rem.lockRemote("LOCK");
      serial_send_update(M2D_DISABLE_DESKTOP);
      serial_send_update(M2D_BOTH_CALIBRATE);
      loadCellAuxiliaryForefootCalibration();
      serial_send_update(M2D_BOTH_CALIBRATE_DONE);
      serial_send_update(M2D_ENABLE_DESKTOP);
      rem.unlockRemote();
      
    // REMOTE COMMANDS BELOW
    case R2M_AUX_STOP:
      aux_stop();
      break;
    case R2M_AUX_AUTO:
      aux_auto();
      break;
    case R2M_MAIN_UP:
      main_up(); 
      break;
    case R2M_MAIN_DOWN:
      main_down(); 
      break; 
    case R2M_MAIN_STOP:
      main_stop();
      break;
    case R2M_MAIN_AUTO:
      main_auto();
      break;
    case R2M_AUX_UP:
      aux_up();    
      break;
    case R2M_AUX_DOWN:
      aux_down();     
      break;
    case R2M_BOTH_TARE:
      both_tare();
      break;
    case R2M_BOTH_HOME:
      both_home();
      break;
    case R2M_BOTH_AUTO:
      both_auto();
      break;

    // SCREEN COMMANDS BELOW
    case S2M_AUX_STOP:
      aux_stop();
      break;
    case S2M_AUX_AUTO:
      aux_auto();
      break;
    case S2M_MAIN_UP:
      main_up(); 
      break;
    case S2M_MAIN_DOWN:
      main_down(); 
      break; 
    case S2M_MAIN_STOP:
      main_stop();
      break;
    case S2M_MAIN_AUTO:
      main_auto();
      break;
    case S2M_AUX_UP:
      aux_up();    
      break;
    case S2M_AUX_DOWN:
      aux_down();     
      break;
    case S2M_MAIN_TARE:
      main_tare();
      break;
    case S2M_MAIN_HOME:
      main_home();
      break;
    case S2M_AUX_TARE:
      aux_tare();
      break;
    case S2M_AUX_HOME:
      aux_home();
      break;
    default: // No input case
      break;
  }

  // Auto Control Algorithm
  if(autoMainRunning)
  {
    if(!auto_state_update(0, loadCellMainBackfootForceNormal(),false)) // Main foot
    {
      linearActuatorMainStop();
      serial_send_update(M2D_MAIN_STOP);
      auto_stop(MAIN_AUTO);
      try_release_all(&lock, MAIN_CONTROL);
    }
  }
  if(autoAuxRunning)
  {
    if(!auto_state_update(1, loadCellAuxiliaryBackfootForceNormal(),false)) // Aux foot
    {
      linearActuatorAuxiliaryStop();
      serial_send_update(M2D_AUX_STOP);
      auto_stop(AUX_AUTO);
      try_release_all(&lock, AUX_CONTROL);
    }
  }

  // Home Control Algorithm
  if(homeMainRunning)
  {
    if(!home_state_update(0, imuMainAngleDegrees(),false)) // Main foot
    {
      linearActuatorMainStop();
      serial_send_update(M2D_MAIN_STOP);
      home_stop(MAIN_HOME);
      try_release_all(&lock, MAIN_CONTROL);
    }
  }
  if(homeAuxRunning)
  {
    if(!home_state_update(1, imuAuxiliaryAngleDegrees(),false)) // Aux foot
    {
      linearActuatorAuxiliaryStop();
      serial_send_update(M2D_AUX_STOP);
      auto_stop(AUX_HOME);
      try_release_all(&lock, AUX_CONTROL);
    }
  }
}

void main_up(void)
{
  home_state_update(MAIN_HOME, 1.0, true);
  auto_state_update(MAIN_AUTO, 1.0, true);
  linearActuatorMainUp(100);
  serial_send_update(M2D_MAIN_UP); 
}

void main_stop(void)
{
  home_state_update(MAIN_HOME, 1.0, true);
  auto_state_update(MAIN_AUTO, 1.0, true);
  linearActuatorMainStop();
  serial_send_update(M2D_MAIN_STOP);
  try_release_all(&lock, MAIN_CONTROL);
}

void main_down(void)
{
  home_state_update(MAIN_HOME, 1.0, true);
  auto_state_update(MAIN_AUTO, 1.0, true);
  linearActuatorMainDown(100);
  serial_send_update(M2D_MAIN_DOWN); 
}

void main_home(void)
{
  auto_state_update(MAIN_AUTO, 1.0, true);
  home_start(MAIN_HOME);
  linearActuatorMainDown(100);
  serial_send_update(M2D_MAIN_HOME);
}

void main_auto(void)
{
  home_state_update(MAIN_HOME, 1.0, true);
  auto_start(MAIN_AUTO);
  linearActuatorMainUp(100);
  serial_send_update(M2D_MAIN_AUTO);
}

void main_tare(void)
{
  auto_state_update(MAIN_AUTO, 1.0, true);
  home_state_update(MAIN_HOME, 1.0, true);
  linearActuatorMainStop();
  loadCellMainForefootTare();
  loadCellMainBackfootTare();
}

void aux_up(void)
{
  home_state_update(AUX_HOME, 1.0, true);
  auto_state_update(AUX_AUTO, 1.0, true);
  linearActuatorAuxiliaryUp(100);
  serial_send_update(M2D_AUX_UP); 
}

void aux_stop(void)
{
  home_state_update(AUX_HOME, 1.0, true);
  auto_state_update(AUX_AUTO, 1.0, true);
  linearActuatorAuxiliaryStop();
  serial_send_update(M2D_AUX_STOP);
  try_release_all(&lock, AUX_CONTROL);
}

void aux_down(void)
{
  home_state_update(AUX_HOME, 1.0, true);
  auto_state_update(AUX_AUTO, 1.0, true);
  linearActuatorAuxiliaryDown(100);
  serial_send_update(M2D_AUX_DOWN); 
}

void aux_home(void)
{
  auto_state_update(AUX_AUTO, 1.0, true);
  home_start(AUX_HOME);
  linearActuatorAuxiliaryDown(100);
  serial_send_update(M2D_AUX_HOME);
}

void aux_auto(void)
{
  home_state_update(AUX_HOME, 1.0, true);
  auto_start(AUX_AUTO);
  linearActuatorAuxiliaryUp(100);
  serial_send_update(M2D_AUX_AUTO);
}

void aux_tare(void)
{
  auto_state_update(AUX_AUTO, 1.0, true);
  home_state_update(AUX_HOME, 1.0, true);
  linearActuatorAuxiliaryStop();
  loadCellAuxiliaryForefootTare();
  loadCellAuxiliaryBackfootTare();
}

void both_tare(void)
{
  auto_state_update(MAIN_AUTO, 1.0, true);
  auto_state_update(AUX_AUTO, 1.0, true);
  home_state_update(MAIN_HOME, 1.0, true);
  home_state_update(AUX_HOME, 1.0, true);
  linearActuatorMainStop();
  linearActuatorAuxiliaryStop();
  loadCellTare();
}

void both_home(void)
{
  auto_state_update(AUX_AUTO, 1.0, true);
  auto_state_update(MAIN_AUTO, 1.0, true);
  home_start(AUX_HOME);
  home_start(MAIN_HOME);
  linearActuatorAuxiliaryDown(100);
  linearActuatorMainDown(100);
  serial_send_update(M2D_AUX_HOME);
  serial_send_update(M2D_MAIN_HOME); 
}

void both_auto(void)
{
  home_state_update(MAIN_HOME, 1.0, true);
  auto_start(MAIN_AUTO);
  linearActuatorMainUp(100);
  serial_send_update(M2D_MAIN_AUTO);
  home_state_update(AUX_HOME, 1.0, true);
  auto_start(AUX_AUTO);
  linearActuatorAuxiliaryUp(100);
  serial_send_update(M2D_AUX_AUTO);
}

// Figure out which command affects which foot (for lock purposes)
// returns integers based on lock library
int command_to_foot(char command)
{
  if(strchr(desktop_main_commands, command) != NULL || strchr(remote_main_commands, command) != NULL || strchr(screen_main_commands, command) != NULL) // if a main foot command
  {
    return MAIN_CONTROL;
  }
  else if(strchr(desktop_aux_commands, command) != NULL || strchr(remote_aux_commands, command) != NULL || strchr(screen_aux_commands, command) != NULL) // if a main foot command
  {
    return AUX_CONTROL;
  }
  return -2; // if invalid command
}

// Figure out which command came from which controller (for lock purposes)
// returns integers based on lock library
int command_to_controller(char command)
{
  if(strchr(desktop_main_commands, command) != NULL || strchr(desktop_aux_commands, command) != NULL) // if a main foot command
  {
    return DESKTOP_CONTROL;
  }
  else if(strchr(remote_main_commands, command) != NULL || strchr(remote_aux_commands, command) != NULL) // if a main foot command
  {
    return REMOTE_CONTROL;
  }
  else if(strchr(screen_main_commands, command) != NULL || strchr(screen_aux_commands, command) != NULL) // if a main foot command
  {
    return SCREEN_CONTROL;
  }
  return -2; // if invalid command
}

void r2m(unsigned int command) {
  char input = ' ';

  switch (command) {
    case RC_SWITCHR:
      input = R2M_BOTH_AUTO;
      break;

    case RC_SWITCHM:
      input = R2M_BOTH_HOME;
      break;

    case RC_SWITCHL:
      input = R2M_BOTH_TARE;
      break;

    case RC_RU:
      input = R2M_MAIN_UP;
      break;

    case RC_RD:
      input = R2M_MAIN_DOWN;
      break;

    case RC_RS:
      input = R2M_MAIN_STOP;
      break;

    case RC_LU:
      input = R2M_AUX_UP;
      break;

    case RC_LD:
      input = R2M_AUX_DOWN;
      break;

    case RC_LS:
      input = R2M_AUX_STOP;
      break;

  }

  if (input != ' ') {
    enqueue(&rbuf, input);
  }
}
