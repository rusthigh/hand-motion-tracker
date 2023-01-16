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

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, 