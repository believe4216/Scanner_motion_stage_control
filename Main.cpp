// This program is to process G-Code text file.
// Extract the contour path information and the infill path information.

#include <fstream>
#include <iostream>
#include "ProcessGcode.h"

using namespace std;



int main()
{

	processGcode();
/*
	int i = initializeScanner();
	if ( i!=0 )
		return i;

	ifstream weldRead;
	string commandStr;
	weldRead.open("./weld.dat");

	while ( !weldRead.eof() ) {
		getline(weldRead, commandStr);
		processCommand(commandStr);
	}

	weldRead.close();

	printf("Finished!\n");
*/
	return 0;
}






