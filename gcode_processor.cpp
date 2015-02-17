// This program is to process G-Code text file.
// Extract the contour path information and the infill path information.

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>

#include "RTC5impl.h"

using namespace std;

const double Del = 1e-6;

const UINT			ResetCompletely(UINT_MAX);
const LONG			R(20000L);

const UINT 			DefaultCard(1U);
const UINT 			LaserMode(0U);
const UINT 			LaserControl(0x18U);

const UINT 			StandbyHalfPeriod(100U*8U);
const UINT 			StandbyPulseWidth(1U*8U);
const UINT 			LaserHalfPeriod(100U*8U);
const UINT 			LaserPulseWidth(50U*8U);
const LONG			LaserOnDelay(100L*1L);
const UINT 			LaserOffDelay(100U*1U);

const UINT 			JumpDelay(250U/10U);
const UINT 			MarkDelay(100U/10U);
const UINT 			PolygonDelay(50U/10U);
const double 		MarkSpeed(250.0);
const double 		JumpSpeed(1000.0);

struct locus		{ long xval, yval; };
const locus BeamDump = { -32000, -32000 };

void 	processTextfile(const string &weldFile, const string &cutFile, const double layerThickness);
void 	processCommand(const string &command);
int 	extractInt(const string commandStr, char cha);
double 	extractDouble(const string commandStr, char cha);

int  initializeScanner(void);
void terminateDLL(void);

int main()
{
	const string weldFile = "./extension_weld.gcode";		// Directory of gcode file for welding
	const string cutFile = "./extension_cut.gcode";		// Directory of gcode file for cutting
	const double layerThickness = 0.1;			// Thickness of each welding layer

	processTextfile(weldFile, cutFile, layerThickness);

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
	terminateDLL();
	return 0;
}

void processTextfile(const string &weldFile, const string &cutFile, const double layerThickness)
{
	ifstream weldRead, cutRead;
	ofstream weldWrite, cutWrite;

	weldRead.open(weldFile);
	cutRead.open(cutFile);
	weldWrite.open("./weld.dat");
	cutWrite.open("./cut.dat");

	string commandStr;
	int layerCounter = 0;

	while ( ! weldRead.eof() ) {
		getline(weldRead, commandStr);

		//Step01: 只保留 G1 开头的命令代码
		if (commandStr[0] != 'G' || 1 != extractInt(commandStr, 'G')) 
			continue;
		//Step02: 丢掉不含 X Y Z 的命令代码
		if ( abs(extractDouble(commandStr, 'X')) < Del &&
				abs(extractDouble(commandStr, 'Y')) < Del &&
				abs(extractDouble(commandStr, 'Z')) < Del )
			continue;
		//Step03: 找到第一层的第一行命令
		if ( layerCounter == 0 && 
			abs(extractDouble(commandStr, 'Z') - layerThickness) > Del)
			continue;
		else if ( layerCounter == 0 ) {
			layerCounter ++;		// 第一层第一个命令已找到
			weldWrite << 'H' << layerCounter << endl;
			weldWrite << commandStr << endl;
		}
		//Step04: 依次找到剩下每层的起点命令，并标注出来
		if ( abs(extractDouble(commandStr, 'Z')) > Del &&
			abs(extractDouble(commandStr, 'Z') - (layerCounter + 1)*layerThickness) > Del )
			continue;
		else if ( abs(extractDouble(commandStr, 'Z') - (layerCounter + 1)*layerThickness) < Del ){
			layerCounter ++;
			weldWrite << 'H' << layerCounter << endl;
		}

		weldWrite << commandStr << endl;
	}
	weldWrite << 'H' << layerCounter + 1 << endl;
	cout << "Final welding layer counter: " << layerCounter << endl;
	if (layerCounter == 0) {
		cout << "Setting of layer thickness is Wrong!!!" << endl;
		return;
	}

	layerCounter = 0;
	while ( ! cutRead.eof() ) {
		getline(cutRead, commandStr);

		//Step01: 只保留 G1 开头的命令代码
		if ( commandStr[0] != 'G' || 1 != extractInt(commandStr, 'G') )
			continue;
		//Step02: 丢掉不含 X Y Z 的命令代码
		if ( abs(extractDouble(commandStr, 'X')) < Del &&
			abs(extractDouble(commandStr, 'Y')) < Del &&
			abs(extractDouble(commandStr, 'Z')) < Del )
			continue;
		//Step03: 找到第一层的第一个命令
		if ( layerCounter == 0 && abs(extractDouble(commandStr, 'Z') - layerThickness) > Del)
			continue;
		else if (layerCounter == 0) {
			layerCounter ++;
			cutWrite << 'H' << layerCounter << endl;
			cutWrite << commandStr << endl;
		}
		//Step04: 依次找到剩下每层的起点命令，并标注出来
		if ( abs(extractDouble(commandStr, 'Z')) > Del &&
			abs(extractDouble(commandStr, 'Z') - (layerCounter + 1)*layerThickness) > Del )
			continue;
		else if ( abs(extractDouble(commandStr, 'Z') - (layerCounter + 1) * layerThickness) < Del ) {
			layerCounter ++;
			cutWrite << 'H' << layerCounter << endl;
		}

		cutWrite << commandStr << endl;
	}
	cutWrite << 'H' << layerCounter + 1 << endl;
	cout << "Final cutting layer counter: " << layerCounter << endl;
	if (layerCounter == 0) {
		cout << "Setting of layer thickness is Wrong!!!" << endl;
		return;
	}

	weldRead.close();
	cutRead.close();
	weldWrite.close();
	cutWrite.close();
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

void processCommand(const string &command)
{
	if ( command[0] == 'H' && 1 != extractInt(command, 'H') ) {
		;//execute_list();		// Execute the list memory of RTC5 board.
	}
	
	cout << command << endl;
}

int  initializeScanner(void)
{
	printf( "Initializing the DLL\n\n" );

	// This function must be called at the very first
	UINT ErrorCode = init_rtc5_dll();

	if ( ErrorCode != 0U ) {
		const UINT QuantityOfRTC5s = rtc5_count_cards();

		if ( QuantityOfRTC5s ) {
			UINT AccError = 0U;

			// Detailed error analysis
			for ( UINT i = 0U; i < QuantityOfRTC5s; i++ ) {
				const UINT Error = n_get_last_error( i );

				if ( Error != 0U ) {
					AccError |= Error;
					printf( "Card no. %u: Error %u detected\n", i, Error );
					n_reset_error( i, Error );
				}
			}

			if ( AccError != 0U ) {
				terminateDLL();
				return 1;
			}
		}
		else {
			printf( "Initializing the DLL: Error %u detected\n", ErrorCode );
			terminateDLL();
			return 1;
		}
	}
	else {
		if ( DefaultCard != select_rtc( DefaultCard ) ) {
			ErrorCode = n_get_last_error( DefaultCard );

			if ( ErrorCode & 256U ) { //  RTC5_VERSION_MISMATCH
				//  In this case load_program_file(0) would not work.
				ErrorCode = n_load_program_file( DefaultCard, 0 );
			}
			else {
				printf( "No acces to card no. %u\n", DefaultCard );
				terminateDLL();
				return 2;
			}

			if ( ErrorCode ) {
				printf( "No access to card no. %u\n", DefaultCard );
				terminateDLL();
				return 2;
			}
			else { //  n_load_program_file was successfull
				(void) select_rtc( DefaultCard );
			}
		}
	}

	set_rtc4_mode();		//  for RTC4 compatibility

	// Initialize the RTC5
	stop_execution();
	//  If the DefaultCard has been used previously by another application 
    //  a list might still be running. This would prevent load_program_file
    //  and load_correction_file from being executed.

    ErrorCode = load_program_file( 0 );     //  path = current working path

    if ( ErrorCode ) {
        printf( "Program file loading error: %u\n", ErrorCode );
        terminateDLL();
        return(3);
    }

    ErrorCode = load_correction_file( 0,   // initialize like "D2_1to1.ct5",
                                      1U,  // table; #1 is used by default
                                      2U); // use 2D only
    if ( ErrorCode != 0U ) {
        printf( "Correction file loading error: %u\n", ErrorCode );
        terminateDLL();
        return(4);
    }

    select_cor_table( 1U, 0U ); //  table #1 at primary connector (default)

    //  stop_execution might have created an RTC5_TIMEOUT error
    reset_error(ResetCompletely);

    printf( "Marking with a fiber laser\n\n" );

    //  Configure list memory, default: config_list(4000,4000)
    config_list(UINT_MAX,           //  use the list space as a single list
                0U);                //  no space for list 2

    set_laser_mode( LaserMode );

    //  This function must be called at least once to activate laser 
    //  signals. Later on enable/disable_laser would be sufficient.
    set_laser_control( LaserControl );

    set_standby( StandbyHalfPeriod, StandbyPulseWidth );

    // Timing, delay and speed preset
    set_start_list( 1U );
        set_laser_pulses( LaserHalfPeriod, LaserPulseWidth );
        set_scanner_delays( JumpDelay, MarkDelay, PolygonDelay );
        set_laser_delays( LaserOnDelay, LaserOffDelay );
        set_jump_speed( JumpSpeed );
        set_mark_speed( MarkSpeed );
    set_end_of_list();

    execute_list( 1U );

    return 0;
}


//  terminateDLL
//
//  Description
//
//  The function waits for a keyboard hit
//  and then calls free_rtc5_dll().
//  

void terminateDLL(void)
{
    printf( "- Press any key to exit.\n" );
    
    while(!_kbhit()) {}
    printf( "\n" );

    free_rtc5_dll();
}
