#ifndef TML_REMOTE_COMMUNICATION_H
#define TML_REMOTE_COMMUNICATION_H

/*
Notes:
- Up, down, and stop, should override home and auto
*/

#define R2M_MAIN_UP 'a'
#define R2M_MAIN_DOWN 'b'
#define R2M_MAIN_STOP 'c'
#define R2M_MAIN_HOME 'd'
#define R2M_MAIN_AUTO 'e'
#define R2M_AUX_UP 'f'
#define R2M_AUX_DOWN 'g'
#define R2M_AUX_STOP 'h'
#define R2M_AUX_HOME 'i'
#define R2M_AUX_AUTO 'j'
#define R2M_BOTH_HOME 'k'
#define R2M_BOTH_AUTO 'l'
#define R2M_BOTH_TARE 'm'

extern char remote_main_commands[];
extern char remote_aux_commands[];

#endif