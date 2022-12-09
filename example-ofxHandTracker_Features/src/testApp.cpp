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
	//tracker2 = new ofxHandTracker(&userGen, &handGen, &depthGen, 1); // use when code is ready to track and handle multiple hands
#endif

#ifdef USE_KINECT
	// demo drawing app setup
	/*drawBuffer.allocate(640, 480);
	prevIndexPos = ofPoint(-1,-1,-1);
	prevMidPos = ofPoint(-1,-1,-1);
	prevPinkyPos = ofPoint(-1,-1,-1);*/
#endif
}

//--------------------------------------------------------------
void testApp::update(){
#ifdef USE_KINECT
	oniContext.update();
	imageGen.update();
	depthGen.update();

	userGen.update(); //crashes app if imageGen not created
	
	try {
		tracker->update();
	}
	catch(const char *e)
	{
		std::cerr << "CRITICAL ERR (update): " << e;
	}
	//tracker2->update();

#endif

	hud.update();
	
	for(int i=0; i<256; i++)
		if(activeKeys[i])
			h.keyPressed(i);
	
	h.update();
	
	// quick box moving demo 
	if(abs(h.getIndexFingerWorldCoord().z - boxPos.z) < 20
	&& abs(h.getIndexFingerWorldCoord().y - boxPos.y) < 20
	&& abs(h.getIndexFingerWorldCoord().x - boxPos.x) < 20){
		boxPos.x = h.getIndexFingerWorldCoord().x;
		boxPos.y = h.getIndexFingerWorldCoord().y;
		boxPos.z = h.getIndexFingerWorldCoord().z;
	}

	int x = ofGetMouseX();
	int y = ofGetMouseY();

	x += hud.getTranslation().x;
	y += hud.getTranslation().y;

	h.origin.x = h.origin.x + ((x - h.origin.x)*0.3f);
	h.origin.y = h.origin.y + ((y - h.origin.y)*0.3f);
}

//--------------------------------------------------------------
void testApp::draw(){

	ofClear(ofColor::black); // prevents uncleared pixels (caused by auto background set to false in setup)

	ofPushMatrix();
	ofTranslate(-hud.getTranslation().x, -hud.getTranslation().y);

#ifdef USE_KINECT

	ofSetColor(ofColor::white);
	try {
		ofPoint hPos = tracker->getHandModel()->origin;
		
		ofPushMatrix();
		ofPushStyle();
		ofSetColor(ofColor::white);
		//ofTranslate(0, ofGetHeight());
		imageGen.draw(640, 0, 640, 480);
		depthGen.draw(0, 0, 640, 480);
		userGen.draw();	
		//handGen.drawHands();
		//ofSetColor(255,255,255);
		tracker->draw();
		ofPopStyle();
		ofPopMatrix();
		
		// this enables user to move around using fist
		if(tracker->getNumFingerTips() == 0) {
			hud.mouseDragged(100*((hPos.x)/640.0)-10, 100*((hPos.y)/480.0)-10, 1);
		}
		
		/*
		if (tracker->get