#ifndef PROCESS_GCODE_H
#define PROCESS_GCODE_H

#include <fstream>
#include <string>
#include "Configuration.h"

using namespace std;

int 	processGcode(void);
int 	extractInt(const string commandStr, char cha);
double 	extractDouble(const string commandStr, char cha);
void 	writeCommand(ofstream &output, string &commandString);

#endif // PROCESS_GCODE_H

