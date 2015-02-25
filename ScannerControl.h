#ifndef SCANNER_CONTROL_H
#define SCANNER_CONTROL_H

#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include "Configuration.h"
#include "RTC5impl.h"

// ---- Initialize the RTC5 Scanner board
int initializeScanner(void);

// ---- Process a line gcode
// execute == 0 : Just write command in list memory
// execute == 1 : Write current command and execute list memory
// mark == 0 : Jump
// mark == 1 : Mark
void loadCommand(int mark, int g_num, double x_num, double y_num, double f_num);

void executeList();

void convertCoords(double x_num, double y_num, double f_num, LONG &x_val, LONG &y_val, double &f_val);

// ---- The function waits for a keyboard hit
// ---- and then calls free_rtc5_dll().
void terminateDLL(void);

#endif // SCANNER_CONTROL_H