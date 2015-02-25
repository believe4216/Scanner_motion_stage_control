#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cmath>
#include "ProcessGcode.h"

using namespace std;

const double DEL = 1.0E-8;

int processGcode()
{
	ifstream weldRead, cutRead;
	ofstream weldWrite, cutWrite;
	string commandString;
	int layerNumber = 0;

	weldRead.open(WELD_GCODE);
	cutRead.open(CUT_GCODE);
	weldWrite.open("./data_welding.txt");
	cutWrite.open("./data_cutting.txt");

	// =============== Process Welding Gcode ==================
	while ( ! weldRead.eof() ) {
		getline(weldRead, commandString);

		// 1: Only keep the GCODE starting with G1, G2 or G3
		if ( commandString[0] != 'G') continue;
		int i = extractInt(commandString, 'G');
		if ( i != 1 && i != 2 && i != 3 ) continue;

		// 2: Only keep the GCODE containing X, Y or Z
		if ( abs(extractDouble(commandString, 'X')) < DEL &&
			abs(extractDouble(commandString, 'Y')) < DEL &&
			abs(extractDouble(commandString, 'Z')) < DEL )
			continue;

		// 3: Find the first Jump command from GCODE
		if ( layerNumber == 0 && 
			abs(extractDouble(commandString, 'X')) + abs(extractDouble(commandString, 'Y')) > DEL )
			writeCommand(weldWrite, commandString);

		// 4: Find the first GCODE of the first layer
		if ( layerNumber == 0 &&
			abs(extractDouble(commandString, 'Z') - LAYER_THICKNESS ) > DEL )
			continue;
		else if ( layerNumber == 0 ) {
			layerNumber ++;
			writeCommand(weldWrite, commandString);
		}

		// 5: Find the first GCODEs in each of the subsequent layers
		if ( abs(extractDouble(commandString, 'Z')) > DEL &&
			abs(extractDouble(commandString, 'Z') - (layerNumber + 1) * LAYER_THICKNESS ) > DEL )
			continue;
		else if (abs(extractDouble(commandString, 'Z') - (layerNumber + 1) * LAYER_THICKNESS ) < DEL ) {
			layerNumber ++;
			weldWrite << -1 << endl; // command for execute list memory
		}

		// 6: Process other ordinary commands
		writeCommand(weldWrite, commandString);
	}
	weldWrite << -1 << endl; // execute command for the last layer

	if ( layerNumber == 0 ) {
		cout << "Setting for layer thickness should be wrong!!!" << endl;
		return 0;    // Failed to process Gcode file	
	}
	cout << "Total welding layer number is: " << layerNumber << endl;


	// ================ Process Cutting Gcode ================
	layerNumber = 0;
	while ( ! cutRead.eof() ) {
		getline(cutRead, commandString);

		// 1: Only keep the GCODE starting with G1 G2 or G3
		if ( commandString[0] != 'G' ) continue;
		int i = extractInt(commandString, 'G');
		if ( i != 1 && i != 2 && i != 3 ) continue;

		// 2: Only keeping the GCODE containing X Y or Z
		if ( abs(extractDouble(commandString, 'X')) < DEL &&
			abs(extractDouble(commandString, 'Y')) < DEL &&
			abs(extractDouble(commandString, 'Z')) < DEL )
			continue;

		// 3: Find the first Jump command from GCODE
		if ( layerNumber == 0 &&
			abs(extractDouble(commandString, 'X')) + abs(extractDouble(commandString, 'Y')) > DEL )
			writeCommand(cutWrite, commandString);

		// 4: Find the first GCODE of the first layer
		if ( layerNumber == 0 &&
			abs(extractDouble(commandString, 'Z') - LAYER_THICKNESS ) > DEL )
			continue;
		else if ( layerNumber == 0 ) {
			layerNumber ++;
			writeCommand(cutWrite, commandString);
		}

		// 5: Find the first GCODEs in each of the subsequent layers
		if ( abs(extractDouble(commandString, 'Z')) > DEL &&
			abs(extractDouble(commandString, 'Z') - (layerNumber + 1) * LAYER_THICKNESS ) > DEL )
			continue;
		else if (abs(extractDouble(commandString, 'Z') - (layerNumber + 1) * LAYER_THICKNESS ) < DEL ) {
			layerNumber ++;
			cutWrite << -1 << endl; // command for execute list memory
		}

		// 6: Process other ordinary gcode
		writeCommand(cutWrite, commandString);
	}
	cutWrite << -1 << endl;

	if ( layerNumber == 0 ) {
		cout << "Setting for layer thickness should be wrong!!!" << endl;
		return 0;    // Failed to process Gcode file
	}
	cout << "Total cutting layer number is: " << layerNumber << endl;

	weldRead.close();
	cutRead.close();
	weldWrite.close();
	cutWrite.close();

	return 1;
}


// Get a integer number following the specified character from a command string
int extractInt(const string commandStr, char cha)
{
	int ind;
	ind = commandStr.find(cha);
	if ( ind < 0 ) return 0;
	return strtol(&commandStr[ind+1], NULL, 10);
}

// Get a 'double' number following the specified character from a command string 
double extractDouble(const string commandStr, char cha)
{
	int ind;
	ind = commandStr.find(cha);
	if ( ind < 0 ) return 0;
	return strtod(&commandStr[ind+1], NULL);
}

void writeCommand(ofstream &output, string &commandString)
{
	// if the command doesn't contain X, Y, return;
	if ( abs(extractDouble(commandString, 'X')) < DEL && abs(extractDouble(commandString, 'Y')) < DEL )
		return;
	// is or isn't a marking command
	if ( abs(extractDouble(commandString, 'E')) > DEL )
		output << 1;	// Marking
	else
		output << 0;	// Jumping

	output  << setw(4) << extractInt(commandString, 'G')
			<< setw(10) << extractDouble(commandString, 'X')
			<< setw(10) << extractDouble(commandString, 'Y')
			<< setw(10) << extractDouble(commandString, 'Z')
			<< setw(10) << extractDouble(commandString, 'F') << endl;
}

