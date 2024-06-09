#ifndef TML_DESKTOP_COMMUNICATION_H
#define TML_DESKTOP_COMMUNICATION_H

#include <HardwareSerial.h>
// #include <USBAPI.h>
#include <Arduino.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

// For limiting number of streams send to desktop
#define STREAM_INTERVAL 1 //7 
// Stream will send to serial once every STREAM_INTERVAL loops.
// 7 is where it starts being ok. May have to increase when there is a busier GUI
extern int stream_limiter; 

/*
COMMUNICATION PACKET STRUCTURE:

Stream:
...

Command/Message:
...

Command/Message (Desktop to MCU):
...

Command States:
  variable = false: command not reading yet
  variable = true: start byte already read, reading command

 Desktop to MCU Command List (can only use 0-9): TODO: make sure up/down/stop overrides home/auto
      0: Stop aux
      1: Aux home-ing procedure start
      2: Aux auto procedure start
      3: Move main up
      4: Move main down
      5: Stop main
      6: Main home-ing procedure start
      7: Main auto procedure start
      8: Move aux up
      9: Move aux down

MCU to Desktop Command List (2 digits max):
      0: Echo (for testing)
      1: Enable Desktop Control
      2: Disable Desktop Control
      3: Main moving up
      4: Main moving down
      5: Main stopping
      6: Main in home-ing procedure
      7: Main in auto procedure
      8: Aux moving up
      9: Aux moving down
      10: Aux Stopping
      11: Aux in home-ing procedure
      12: Aux in auto procedure
      13: Both in home-ing procedure
      14: Both in auto procedure

*/

#define D2M_AUX_STOP '0'
#define D2M_AUX_HOME '1'
#define D2M_AUX_AUTO '2'
#define D2M_MAIN_UP '3'
#define D2M_MAIN_DOWN '4'
#define D2M_MAIN_STOP '5'
#define D2M_MAIN_HOME '6'
#define D2M_MAIN_AUTO '7'
#define D2M_AUX_UP '8'
#define D2M_AUX_DOWN '9'
#define D2M_BOTH_CALIBRATE '!'
#define D2M_MAIN_HEEL_CALIBRATE 'w'
#define D2M_MAIN_FORE_CALIBRATE 'v'
#define D2M_AUX_HEEL_CALIBRATE 'u'
#define D2M_AUX_FORE_CALIBRATE 't'
#define D2M_BOTH_TARE 'z'
#define D2M_MAIN_TARE 'y'
#define D2M_AUX_TARE 'x'

extern char desktop_main_commands[];
extern char desktop_aux_commands[];

#define M2D_ECHO "0"
#define M2D_ENABLE_DESKTOP "1"
#define M2D_DISABLE_DESKTOP "2"
#define M2D_MAIN_UP "3"
#define M2D_MAIN_DOWN "4"
#define M2D_MAIN_STOP "5"
#define M2D_MAIN_HOME "6"
#define M2D_MAIN_AUTO "7"
#define M2D_AUX_UP "8"
#define M2D_AUX_DOWN "9"
#define M2D_AUX_STOP "10"
#define M2D_AUX_HOME "11"
#define M2D_AUX_AUTO "12"
#define M2D_BOTH_HOME "13"
#define M2D_BOTH_AUTO "14"
#define M2D_BOTH_CALIBRATE "15"
#define M2D_BOTH_CALIBRATE_DONE "16"

#define STREAM_START ">"
#define COMMAND_START "~"
#define COMMAND_END "<"
#define SEPARATOR "|"

/*
* FUNCTION: round2(num)
* 
* returns num but rounded to 2 decimal places
*
* num = double type number
* returns: double type rounded number of num
*/
double round2(double num);

/*
* FUNCTION: serial_send_message(serial, msg)
* 
* sends a command or message STRING through serial using the above communication packet structure
*
* serial = always input {Serial}
* msg = string to send (ex: "7", "11", "There is an error")
* returns: void
*/
void serial_send_update(const char* msg);

/*
* FUNCTION: serial_send_message(serial, c)
* 
* sends a stream packet through serial using the above communication packet structure
*
* serial = always input {Serial}
* ma = main foot angle reading
* mh = main foot heel reading
* mf = main foot fore reading
* aa = aux foot angle reading
* ah = aux foot heel reading
* af = aux foot fore reading
* returns: void
*/
void serial_send_stream(double ma, double mh, double mf, double aa, double ah, double af);

/*
* FUNCTION: getSerialInput(void)
* 
* Reads serial character, and depending on the character, 
* return a number corresponding to the command, if applicable.
* This function does not take packets and data structure into account, just the character
*
* Usage: 
```
input = getSerialInput
if(input != ' '){
  switch(input){
    ...
  }
}
```
* 
* returns: command char; ' ' if no command yet
*/
char getSerialInput(void);

/*
* FUNCTION: ... #FIXME: INCOMPLETE FUNCTION
* 
* Reads serial character, and depending on the character and command state, 
* return a number corresponding to the command, if applicable.
* This function does take packets and data structure into account
* 
* *state = address of variable that holds the command state
* *data = address of data that is going to be written to
* returns: command number; -1 if no command
*/
int getSerialPacketInput(bool *state, int *data);

#endif