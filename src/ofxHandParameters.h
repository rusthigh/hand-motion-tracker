#pragma once

// TODO: include safety defined macros in all project files (also define smart way of naming them)
// UPDATE: #pragma once does the same thing as this macro i suppose
//#ifndef _Hand_Tracker_Hand_Model_Parameters
//#define _Hand_Tracker_Hand_Model_Parameters

#include "ofMain.h"

// left and right thumb swing
#define THUMB_MIN_ANGLE_X		 -40//-30 -> less opened
#define THUMB_MAX_ANGLE_X	      10//0 

// front and back thumb swing
#define THUMB_MIN_ANGLE_Z		   0
#define THUMB_MAX_ANGLE_Z		  20

// define other fingers fr