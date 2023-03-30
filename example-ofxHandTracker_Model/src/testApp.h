// @author: blazm
// @about: this example works without kinect, just to see kinematic hand model in action 

#pragma once

#include "ofMain.h"
#include "ofxHandModel.h"
// these are not needed in this example
//#include "ofxHandTracker.h" // ofxHandTracker in this project was also excluded from addons,
							  // cause ofxOpenNI and ofxOpenCV were not loaded

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
		void mou