#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	boxPos = h.origin - ofPoint(-ofGetWidth()/2, 0, 0);

	for(int i=0; i<256; i++)
		activeKeys[i] = false;

    ofSetBackgroundAuto(false); // removes fullscreen flickering (on some PCs)
	ofBackground(ofColor::black);

	//hud = HUD(ofPoint(10,10,0), ofColor(64, 128, 96, 128), 100, 100);
	//hud = HUD(ofPoint(10, ofGetHeight() - ofGetHeight()/2 - 20, 0));
	hud = HUD(ofPoint(10,10,0), 120, 60, 2.0f, 2.0f); // init hud with number of pages - always locks number of pages (no matter of resolution)
	//hud = HUD(ofPoint(10, 870, 0), 250, 150, 2000, 7000); // init hud with locked resolution to be available (you want 10k x 8k res or even more? fine, just set it up)
	//hud = HUD(ofPoint(0, 0, 0), 100, 1000, 1000, 10000); // setup for ppt
	//hud = HUD(ofPoint(0, 0, 0), 1, 1, 1.0f, 1.0f);
	hud.setColor(ofColor(64, 128, 96, 64), ofColor(255, 96, 32, 150));
	
	h.origin.x = ofGetWidth()/2;
	h.origin.y = ofGetHeight()/2;

	//ofSetFullscreen(true);
	ofSetFrameRate(30);
	ofSetVerticalSync(true);

	wasGrabbed = false;
	wasReleased = true;

#ifdef USE_KINECT
	oniContext.setup();
	//oniContext.setMirror(true);

	depthGen.setup(&oniContext);
	imageGen.setup(&oniContext);

	userGen.setup(&oniContext);
	userGen.setSmoothing(0.1);				// built in openni skeleton smoothing...
	//userGen.setUseMaskPixels(true);
	userGen.setUseCloudPoints(true); // must be enabled to be able to draw cloud points
	userGen.setMaxNumberOfUsers(2);					// use this to set dynamic max number of users (NB: that a hard upper limit is defined by MAX_NUMBER_USERS in ofxUserGenerator)
	
	handGen.setup(&oniContext, 2);	
	handGen.setSmoothing(0.02);

	oniContext.toggleRegisterViewport();
	oniContext.toggleMirror();

	tracker = new ofxHandTracker(&userGen, &handGen, &depthGen, 0);
	//tracker2 = new ofxHandTracker(&userGen, &handGen, &depthGen, 1); // use whe