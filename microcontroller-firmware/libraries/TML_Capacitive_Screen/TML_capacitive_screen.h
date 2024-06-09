// TML_Capacitive_Screen.h

// Enqueue these characters into the centralized buffer to make the device perfom an action
#define S2M_MAIN_UP 'n'
#define S2M_MAIN_DOWN 'o'
#define S2M_MAIN_STOP 'p'
#define S2M_MAIN_HOME 'q'
#define S2M_MAIN_AUTO 'r'
#define S2M_MAIN_TARE '^'
#define S2M_AUX_UP 's'
#define S2M_AUX_DOWN '@'
#define S2M_AUX_STOP '#'
#define S2M_AUX_HOME '$'
#define S2M_AUX_AUTO '%'
#define S2M_AUX_TARE '+'

extern char screen_main_commands[];
extern char screen_aux_commands[];

// Setup
void displayWireSetup(void);
void displaySetup(void);

// Display Functions
char displayTouchChecker(void);
void displayUpdate(int main_fore, int main_heel, int main_angle, int aux_fore, int aux_heel, int aux_angle);
void displayDrawText(char16_t x, char16_t y, char16_t color, char16_t size, char* text);
void displayDrawForce(char16_t x, char16_t y, char16_t color, char16_t size, int force);
void displayDrawAngle(char16_t x, char16_t y, char16_t color, char16_t size, int angle);
void displayDrawLine(char16_t x1, char16_t y1, char16_t x2, char16_t y2, char16_t color);
void displayDrawBox(char16_t x1, char16_t y1, char16_t x2, char16_t y2, char16_t color, char16_t radius);
void displayClearBox(char16_t x1, char16_t y1, char16_t x2, char16_t y2, char16_t color);

// Display Library Functions
void displayTestSetup(void);
void displayTestLoop(void);

unsigned long testFillScreen(void);
unsigned long testText(void);
unsigned long testLines(char16_t color);
unsigned long testFastLines(char16_t color1, char16_t color2);
unsigned long testRects(char16_t color);
unsigned long testFilledRects(char16_t color1, char16_t color2);
unsigned long testFilledCircles(char16_t radius, char16_t color);
unsigned long testCircles(char16_t radius, char16_t color);
unsigned long testTriangles(void);
unsigned long testFilledTriangles(void);
unsigned long testRoundRects(void);
unsigned long testFilledRoundRects(void);