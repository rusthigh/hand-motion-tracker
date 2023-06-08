#pragma once

//#include "ofMain.h"
#include "ofxHandModel.h"
#include "ofxOpenNI.h"
#include "ofxOpenCv.h"
#include "ofxImageMatcher.h"

#define	TINY_IMG_DIM		150
#define FTIP_HIST_SIZE		3	// history array size

#define	HAND_GRABBED			"hand_grabbed"
#define HAND_RELEASED			"hand_released"
#define HAND_PICKED				"hand_picked"

#define MIN_HAND_DEPTH		 500.0
#define MAX_HAND_DEPTH	    2000.0

// tracker for each hand
class ofxHandTracker
{
	public:
		//ofxHandTracker();
		ofxHandTracker(ofxUserGenerator *_userGen, ofxHandGenerator 