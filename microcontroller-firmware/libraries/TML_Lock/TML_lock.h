#ifndef TML_LOCK_H
#define TML_LOCK_H

#include <stdio.h>

#define DESKTOP_CONTROL 1
#define AVAILABLE 0
#define REMOTE_CONTROL -1
#define SCREEN_CONTROL -2
#define MAIN_CONTROL 2
#define AUX_CONTROL 3

// Structure for Lock
typedef struct {
  // These values will be either:
  // - DESKTOP_CONTROL
  // - AVAILABLE
  // - REMOTE_CONTROl
  int main_control = 0;
  int aux_control = 0;
} Lock;

/*
* FUNCTION: check_lock(Lock *lock, int foot)
* 
* returns of control state of the foot specified.
* Input MAIN_CONTROL for main, and AUX_CONTROL for aux
*
* *lock = address of Lock object
* foot = MAIN_CONTROL or AUX_CONTROL
* returns: value of main_control or aux_control, which represents whether desktop or remote have control
*   which will be DESKTOP_CONTROL, AVAILABLE, or REMOTE_CONTROl
*/
int check_lock(Lock *lock, int foot);

/*
* FUNCTION: try_acquire(Lock *lock, int foot, int controller)
* 
* "controller" (DESKTOP_CONTROL or REMOTE_CONTROL) tries to acquire control of the platform specified by foot
* if success, return true. If failed (already acquired by the other) return false
*
* *lock = address of Lock object
* foot = MAIN_CONTROL or AUX_CONTROL
* controller = DESKTOP_CONTROL or REMOTE_CONTROL, whichever is requesting control
* returns: whether acquiring was succesful or not
*/
bool try_acquire(Lock *lock, int foot, int controller);

/*
* FUNCTION: try_release(Lock *lock, int foot, int controller)
* 
* "controller" (DESKTOP_CONTROL or REMOTE_CONTROL) tries to release control of the platform specified by foot
* if success, return true. If failed (already released) return false
*
* *lock = address of Lock object
* foot = MAIN_CONTROL or AUX_CONTROL
* controller = DESKTOP_CONTROL or REMOTE_CONTROL, whichever is requesting to release
* returns: whether releasing was succesful or not. Return false if inputs are invalid (-2)
*/
bool try_release(Lock *lock, int foot, int controller);

/*
* FUNCTION: try_release_all(Lock *lock, int foot)
* 
* tries to release control of the platform specified by foot, for both controllers
* if success, return true. If failed (already released) return false
*
* *lock = address of Lock object
* foot = MAIN_CONTROL or AUX_CONTROL
* returns: whether releasing was succesful or not, for both (false if either is already released). Return false if inputs are invalid (-2)
*/
bool try_release_all(Lock *lock, int foot);

#endif