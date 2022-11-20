#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	boxPos = h.origin - ofPoint(-ofGetWidth()/2, 0, 0);

	for(int i=0; i<256; i++)
		activeKeys[i] = false;

    ofSetBackgroundAuto(false); // remove