#ifndef TML_CONTROL_ALGORITHMS_H
#define TML_CONTROL_ALGORITHMS_H

#include <HardwareSerial.h>
#include <Arduino.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define MAIN_AUTO 0
#define AUX_AUTO 1

#define MAIN_HOME 0
#define AUX_HOME 1

// Global variables for keeping track of auto modes
extern bool autoMainRunning;
extern bool autoAuxRunning;

// Global variables for auto stopping threshold
extern double stopThreshold_N;

// Global variables for keeping track of home modes
extern bool homeMainRunning;
extern bool homeAuxRunning;

// Definition for home angle threshold
#define HOME_ANGLE 1.0 // deg

/*
* FUNCTION: auto_start(int foot)
* 
* Starts auto process for the specified foot by changing a boolean representing whether auto is running
* !!! Would need to start moving the actuator up through a separate function call (actuator library)
*
* foot = 0 for main, 1 for aux
*/
void auto_start(int foot);

/*
* FUNCTION: auto_stop(int foot)
* 
* Stops auto process for the specified foot by changing a boolean representing whether auto is running
* !!! Would need to stop moving the actuator up through a separate function call (actuator library)
*
* foot = 0 for main, 1 for aux
*/
void auto_stop(int foot);

/*
* FUNCTION: auto_state_update(int foot, double force, bool override)
* 
* Updates whether the auto process should keep going or whether it should stop
* Might be a good idea to call this multiple times to make sure it is checking as often as possible
* If auto bool is already false, return false
*
* foot = 0 for main, 1 for aux
* force_foot = load cell heel force for correct foot
* override = true when we want to stop auto process immediately
* returns: true if actuator should keep running, false if auto process is to stop
*/
bool auto_state_update(int foot, double force, bool override);

/*
* FUNCTION: home_start(int foot)
* 
* Starts home process for the specified foot by changing a boolean representing whether home is running
* !!! Would need to start moving the actuator down through a separate function call (actuator library)
*
* foot = 0 for main, 1 for aux
*/
void home_start(int foot);

/*
* FUNCTION: home_stop(int foot)
* 
* Stops home process for the specified foot by changing a boolean representing whether home is running
* !!! Would need to stop moving the actuator down through a separate function call (actuator library)
*
* foot = 0 for main, 1 for aux
*/
void home_stop(int foot);

/*
* FUNCTION: home_state_update(int foot, double force, bool override)
* 
* Updates whether the home process should keep going or whether it should stop
* Might be a good idea to call this multiple times to make sure it is checking as often as possible
* If home bool is already false, return false
*
* foot = 0 for main, 1 for aux
* angle = angle for correct foot
* override = true when we want to stop home process immediately
* returns: true if actuator should keep running, false if home process is to stop
*/
bool home_state_update(int foot, double angle, bool override);

#endif