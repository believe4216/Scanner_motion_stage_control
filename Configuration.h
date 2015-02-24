#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <conio.h>
#include "RTC5impl.h"
using namespace std;

// Strings weldGcode and cutGcode give the path of two gcode files.
// The two gcode files should be built by Skeinforge or Slicr3 software.
// The two files are all input for our program.
const string WELD_GCODE = "./weld.gcode";
const string CUT_GCODE  = "./cut.gcode";
const double LAYER_THICKNESS = 0.1;		// mm, thickness of each layer

//=========================================================================
//================================= Scanner ===============================
//=========================================================================

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


#endif //CONFIGURATION_H