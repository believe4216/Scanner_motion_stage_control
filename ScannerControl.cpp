
#include <windows.h>
#include "ScannerControl.h"

using namespace std;

// ---- Initialize the RTC5 Scanner board
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

    // Wait for the execution of list 1 finishing, and to be vailable again.
    while ( !load_list(1U, 0U) ) ;
    //Sleep(100);


    return 0;
}

// ---- Process a line gcode
// mark == 0 : Jump
// mark == 1 : Mark
void loadCommand(int mark, int g_num, double x_num, double y_num, double f_num)
{
	// Convert the coords in GCODE to the coords for SCANNER
	LONG x_val, y_val;
	double f_val;
	convertCoords(x_num, y_num, f_num, x_val, y_val, f_val);

	if ( mark == 0 ) {			// Jump Command
		jump_abs( x_val, y_val );
		printf( "J" );
	}
	else {
		// Check marking speed:
		// if marking speed changed, set a new marking speed
		if ( g_num == 1 )		// Mark lines
			mark_abs( x_val, y_val);
		if ( g_num == 2 )		// Mark Arcs
			;
		if ( g_num == 3 )		// Mark Arcs
			;
		printf( "M" );
	}

}

// Execute list memory (Only use list 1)
void executeList(void)
{
	set_end_of_list();
	execute_list(1U);
	printf( "X" );
	//Sleep(100);
	// Wait for the execution of list being finished, and being available again
	while ( !load_list(1U, 0U ) );
}

void convertCoords(double x_num, double y_num, double f_num, LONG &x_val, LONG &y_val, double &f_val)
{
	x_val = (long)( 500 * x_num );
	y_val = (long)( 500 * y_num );
	f_val = f_num;
}

// ---- The function waits for a keyboard hit
// ---- and then calls free_rtc5_dll().
void terminateDLL(void)
{
	printf( "- Press any key to exit.\n" );
    
    while(!_kbhit()) {}
    printf( "\n" );

    free_rtc5_dll();
}