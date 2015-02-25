// This program is to process G-Code text file.
// Extract the contour path information and the infill path information.

#include <fstream>
#include <iostream>
#include "ProcessGcode.h"
#include "ScannerControl.h"

using namespace std;

int cuttingProcess(ifstream &mStageData);

int main()
{
	// Process two GCODE files.
	// Obtain two files containing useful numerical data.
	processGcode();

	// Initialize RTC5 control board.
	// Terminate the program if the initialization fails.
	int i = initializeScanner();
	if ( i!=0 ) return i;
	;						// Initialize the control of Motion Stage

	// Define two object to input processing data
	ifstream scannerData, mStageData;
	scannerData.open("./data_welding.txt");
	mStageData.open("./data_cutting.txt");

	int workType, gNum;
	double xNum, yNum, zNum, fNum;

	int count = 0;									// Just for test ~ ~ ~ ~ ~ ~ ~
	while ( ! scannerData.eof() && count < 10000 ) {
		scannerData >> workType;
		if ( workType == -1 ) {		// one layer's commands loading complete, execute list memory
			executeList();

			if ( cuttingProcess(mStageData) ) {   // invoke the cutting process
				cout << "Error occurs in cutting Process !!!\n";
				break;
			}
			continue;				// Start next layer
		}
		// extract and load command into list memory
		scannerData >> gNum >> xNum	>> yNum >> zNum >> fNum;
		loadCommand(workType, gNum, xNum, yNum, fNum);
		count ++;
	}

	scannerData.close();
	mStageData.close();

	printf("Job Finished!\n");

	terminateDLL();		// Release the control of Scanner
	;					// Release the control of Motion stage

	return 0;
}

int cuttingProcess(ifstream &mStageData)
{
	if ( mStageData.eof() ) return 1;
	int count = 0;
	while ( !mStageData.eof() && count < 100 ) {
		cout << "C";
		count ++;
	}
	cout << endl;
	return 0;	// 0 success, 1 fail
}






