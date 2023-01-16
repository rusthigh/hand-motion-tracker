// @author: blazm
// @about: this example requires kinect, showing features of hand tracker class

#pragma once

#include "ofMain.h"
#include "ofxHandModel.h"
#include "ofxHandTracker.h"
#include "ofxOpenNI.h"
#include "ofxOpenCv.h"
#include "HUD.h"

#define USE_KINECT
#define NUM_DEMO_PARAMS_X		6
#define NUM_DEMO_PARAMS_Y		4

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void ke