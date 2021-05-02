#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	for(int i=0; i<256; i++)
		activeKeys[i] = false;

	ofSetBackgroundAuto(false); // removes fullscreen flickering (on some PCs)
	ofBackground(ofColor::white);

	h.origin.x = ofGetWidth()/2;
	h.origin.y = ofGetHeight()/2;

	//ofSetFullscreen(true);
	ofSetFrameRate(30);
	ofSetVerticalSync(true);

	wasGrabbed = false;
	wasReleased = true;

#ifdef USE_KINECT
	oniC