/*
NOTE: Serial will appear with a squiggly line if opened in VSCODE.
This does not mean anything. The code will still work
*/

#include "TML_desktop_communication.h"

int stream_limiter = 0; 
char desktop_main_commands[] = "34567!zytuvw";
char desktop_aux_commands[] = "01289!zxtuvw";

double round2(double num) {
    return round(num * 100.0) / 100.0;
}

void serial_send_update(const char* msg)
{
  if(Serial)
  {
    Serial.print(COMMAND_START);
    Serial.print(msg);
    Serial.print(COMMAND_END);
    return;
  }
}

void serial_send_stream(double ma, double mh, double mf, double aa, double ah, double af) // TODO: double data type, correct?
{
  /*
   * 2 options:
   * - One function that sends whole stream at once (easier)
   * - Function sends one character at a time, and there's a pointer parameter that keeps track of what part of the stream we are at
   *   (increases response time of update sending, but only by however many lines are in this function?)
   */

  stream_limiter++;
  if(stream_limiter >= STREAM_INTERVAL)
  {
    Serial.print(STREAM_START);
    Serial.print(round2(ma));
    Serial.print(SEPARATOR);
    Serial.print(round2(mh));
    Serial.print(SEPARATOR);
    Serial.print(round2(mf));
    Serial.print(SEPARATOR);
    Serial.print(round2(aa));
    Serial.print(SEPARATOR);
    Serial.print(round2(ah));
    Serial.print(SEPARATOR);
    Serial.print(round2(af));
    Serial.print(SEPARATOR);

    stream_limiter = 0;
  }

  return;
}

char getSerialInput(void)
{
    if(Serial.available())
    {
        return Serial.read();
    }
    return '\0'; // Null character
}

int getSerialPacketInput(bool *state, int *data)
{
    // TODO: work in progress
    // recognize start/end chars and change state accordingly
    // Only return a command number at end char, otherwise, return -1
    // ^ this is so that the program doesn't mistakenly use the "data" value while it is still being processed
    // Somehow convert a series of characters into an integer (rolling. keep editing data by adding place values or something)
    char c;
    if(Serial.available())
    {
        c = Serial.read();
    }
    return -1; // Empty character
}