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

// define other fingers front and back swing limits (actual angle of first segment, value is then propagated to others)
#define FINGER_MIN_ANGLE_Z	 0
#define FINGER_MAX_ANGLE_Z	90

// also need to define non-thumb finer x angles -> but they are different for each finger
#define FINGER_MIN_ANGLE_X -10
#define FINGER_MAX_ANGLE_X	10

// used to store local finger parameters
class ofxFingerParameters
{
	public:
		ofxFingerParameters() {
			fx1 = 0;
			fx2 = fx1;
			fx3 = fx2;
			fx4 = fx3;

			fz1 = FINGER_MAX_ANGLE_Z;
			fz2 = FINGER_MAX_ANGLE_Z;
			fz3 = FINGER_MAX_ANGLE_Z;
			fz4 = FINGER_MAX_ANGLE_Z;

			params = 0;

			tx = 0; 
			tz = 0