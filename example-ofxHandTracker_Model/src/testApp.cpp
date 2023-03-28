
#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	// setup multi keys flags
	for(int i=0; i<256; i++)
		activeKeys[i] = false;

    ofSetBackgroundAuto(false); // removes fullscreen flickering (on some PCs)
	ofBackground(ofColor::black);

	h.origin.x = ofGetWidth()/2;
	h.origin.y = ofGetHeight()/2;

	ofSetFrameRate(60);
	ofSetVerticalSync(true);

	wasGrabbed = false;
	wasReleased = true;

	// model & demo params setup
	for(int i=0; i<NUM_DEMO_PARAMS_X; i++) {
		for(int j=0; j<NUM_DEMO_PARAMS_Y; j++) {
	
		demoParams[i][j] = ofxFingerParameters(ofRandom(FINGER_MIN_ANGLE_Z, FINGER_MAX_ANGLE_Z),
										   ofRandom(FINGER_MIN_ANGLE_Z, FINGER_MAX_ANGLE_Z),
										   ofRandom(FINGER_MIN_ANGLE_Z, FINGER_MAX_ANGLE_Z),
										   ofRandom(FINGER_MIN_ANGLE_Z, FINGER_MAX_ANGLE_Z),