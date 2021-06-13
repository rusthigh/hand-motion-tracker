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

	thesisPresentation.update();

	for(int i=0; i<256; i++)
		if(activeKeys[i])
			h.keyPressed(i);
	
	h.update();

	int x = ofGetMouseX();
	int y = ofGetMouseY();

	h.origin.x = h.origin.x + ((x - h.origin.x)*0.3f);
	h.origin.y = h.origin.y + ((y - h.origin.y)*0.3f);
}

//--------------------------------------------------------------
void testApp::draw(){
	
	ofClear(ofColor::white); // prevents uncleared pixels (caused by auto background set to false in setup)

	ofPushMatrix();
	
	thesisPresentation.draw();

#ifdef USE_KINECT

	ofSetColor(ofColor::white);
	try {
		ofPoint hPos = tracker->getHandModel()->origin; // position in boundary dimensions 640x480

		float wRatioKinect = ofGetWidth()/640.0;
		float hRatioKinect = ofGetHeight()/480.0;
		ofPoint dispPos = ofPoint(hPos.x * wRatioKinect, hPos.y * hRatioKinect, hPos.z);

		/*if(thesisPresentation.hasFinished()) {	
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
		}*/
		/*if(tracker->getNumFingerTips() == 0) {
			hud.mouseDragged(100*((hPos.x)/640.0)-10, 100*((hPos.y)/480.0)-10, 1);
		}*/
		
		/*ofPushStyle();
		ofSetColor(0,0,0);
		ofDrawBitmapString("PARAMS: " + ofToString(tracker->getHandModel()->saveFingerParameters().params), 100, 100);
		ofPopStyle();*/

		/*if (tracker->getHandModel()->saveFingerParameters().params == 16) {
			thesisPresentation.slideToPage(-1);
		}
		else if (tracker->getHandModel()->saveFingerParameters().params == 6) {
			thesisPresentation.slideToPage(1);
		}
		*/
		if (tracker->getNumFingerTips() == 1) {
			if(!wasGrabbed && dispPos.distance(ofPoint(0,0,0)) > 5 &&
				(dispPos.x > ofGetWidth()*0.25 && dispPos.x < ofGetWidth()*0.75)) {
				wasGrabbed = true;
				wasReleased = false;
				pointGrabbed = dispPos;
			}
		}
		else {
			if(!wasReleased) {
				wasReleased = true;
				wasGrabbed = false;

				if(abs(pointGrabbed.x - pointReleased.x) > ofGetWidth()/10) {
					if(pointGrabbed.x > pointReleased.x)
						thesisPresentation.slideToPage(-1);
					else
						thesisPresentation.slideToPage(1);
				}

				pointGrabbed = ofPoint(0,0,0);
				pointReleased = pointGrabbed;
			}
		}

		if (wasGrabbed) {
			pointReleased = dispPos;
			if (pointReleased.distance(pointGrabbed) > ofGetWidth()/2) {
				wasGrabbed = false;
				wasReleased = true;
				
				pointGrabbed = ofPoint(0,0,0);
				pointReleased = pointGrabbed;
			}
		}

		ofPushStyle();
		ofFill();
		ofSetLineWidth(4.0);
		ofSetColor(120, 120, 120, 64);
		ofLine(pointGrabbed, pointReleased);
		//ofDrawArrow(pointGrabbed, pointReleased, 10.0f);
		ofPopStyle();


		ofEnableAlphaBlending();
		ofPushMatrix();
		//ofScale(1.5, 1.5);
		//ofTranslate(hud.getTranslation().x, hud.getTranslation().y);
		ofPushStyle();
		//ofSetLineWidth(3.0);
		//ofNoFill();
		ofSetColor(128, 255, 0);
		//ofCircle(dispPos, 50);
		//ofSetColor(255, 240, 100);
		tracker->getHandModel()->getProjection().draw(dispPos - ofPoint(75, 75));
		ofPopStyle();
		ofPopMatrix();
		ofDisableAlphaBlending();
	}
	catch(const char *e)