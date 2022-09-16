// @author blazm
// @about this example works without kinect, just to see kinematic hand model and demo presentation in action

#pragma once

#include "ofMain.h"
#include "ofxHandModel.h"
// these are not needed in this example
//#include "ofxHandTracker.h"
//#include "ofxOpenNI.h"
//#include "ofxOpenCv.h"
#include "Presentation.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int