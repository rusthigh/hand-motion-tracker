
#include "ofxHandTracker.h"

/*ofxHandTracker::ofxHandTracker()
{
	userGen = NULL;
	handGen = NULL;
}*/

ofxHandTracker::ofxHandTracker(ofxUserGenerator *_userGen, ofxHandGenerator *_handGen, ofxDepthGenerator *_depthGen, int _hIndex)
{
	userGen = _userGen;
	handGen = _handGen;
	depthGen = _depthGen;

	hIndex = _hIndex;
	h.scaling = ofPoint(.3,.3,.35);

	// setup images to be used for tracking real hand
	realImg.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE); 
	realImg.setUseTexture(true);

	blankImg.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE); 
	blankImg.setUseTexture(true);
	//colorImg.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_COLOR); 
	//colorImg.setUseTexture(true);

	for (int i=0; i<IMG_DIM; i++) {    
		for (int j=0; j<IMG_DIM; j++) {    
			realImg.setColor(i, j, ofColor::black);    // ofColor(ofRandom(255), ofRandom(255), ofRandom(255))
			blankImg.setColor(i, j, ofColor::black);
		}    
	}

	realImgCV.allocate(IMG_DIM, IMG_DIM);
	realImgCV_previous.allocate(IMG_DIM, IMG_DIM);

	// setup model images
	modelImg.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE);  
	for (int i=0; i<IMG_DIM; i++) {    
		for (int j=0; j<IMG_DIM; j++) {    
			modelImg.setColor(i, j, ofColor::black);    // ofColor(ofRandom(255), ofRandom(255), ofRandom(255))
		}    
	}

	modelImgCV.allocate(IMG_DIM, IMG_DIM);

	// setup contour & difference image
	diffImg.allocate(IMG_DIM,IMG_DIM);
	diffImg.set(0);

	palmCenter = ofPoint(IMG_DIM/2, IMG_DIM/2, 0);
	palmRadius = 0;

	rollAngle = 0;

	fingerTipsCounter = 0;

	// logging number of detected fingertips for few steps back
	fTipLastInd = 0;
	for (int i=0; i<FTIP_HIST_SIZE; i++) {
		fTipHistory[i] = 0;
	}
	/*
	ofFbo::Settings s = ofFbo::Settings();  
	s.width = IMG_DIM;  
	s.height = IMG_DIM;  
	s.internalformat = GL_LUMINANCE32F_ARB;
	depthFbo.allocate(s);*/
	depthFbo.allocate(IMG_DIM, IMG_DIM);

	//tinyHandImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM, OF_IMAGE_GRAYSCALE);
	//tinyModelImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM, OF_IMAGE_GRAYSCALE);
	tinyHandImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM);
	tinyModelImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM);
	tinyDiffImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM);

	//ofxFingerParameters p = ofxFingerParameters(30);
	//fingerTips.resize(5);

	shaderMatcher.setup(IMG_DIM, IMG_DIM);
}

void ofxHandTracker::fetchHandPointCloud(ofPoint _handTrackedPos) {
	int width = userGen->getWidth();
	int height = userGen->getHeight();
		/*
	int xx = handGen->getHand(handIndex)->projectPos.x;
	int yy = handGen->getHand(handIndex)->projectPos.y;
	int zz = handGen->getHand(handIndex)->projectPos.z;
	*/
	int xx = _handTrackedPos.x;
	int yy = _handTrackedPos.y;
	int zz = _handTrackedPos.z;
	int raw = _handTrackedPos.z;
	ofPoint handTrackedPos = _handTrackedPos;

	ofPoint handImageTrackedPos = handTrackedPos;
	handImageTrackedPos.z = 0;

	float percentage = getHandDepth(raw, true);

	//depthGen->setDepthThreshold(0, 600, 800);
	//depthGen->update();

	// here's the part where we segment hand from depthGenerator (not used but should be available to other devs as feature)
	// bad thing here is that we dont get much depth information, so its just silhouette of the hand
	/*unsigned char *pix = depthGen->getDepthPixels(raw - 100, raw + 100, 500, 640, 640);
	ofImage depth;
	depth.setFromPixels(pix, 640, 480, ofImageType::OF_IMAGE_GRAYSCALE);

	depthFbo.begin();
		ofClear(ofColor::black);
		ofPushMatrix();
		ofScale(0.8*(1 + percentage), 0.8*(1 + percentage));
		ofTranslate(-handImageTrackedPos + ofPoint((IMG_DIM/2)*(1 - percentage/2), (IMG_DIM/2)*(1 - percentage/2), 0));
		//depthGen->draw();
		depth.draw(0,0);
		ofPopMatrix();
	depthFbo.end();
	*/
	// bounding square limits
	int bbMinX = 160; 
	int bbMaxX = 160;
	int bbMinY = 160;
	int bbMaxY = 160;

	// adjust bb by distance factor
	if (zz >= 500) {
		bbMinX -= distFactor(zz);
		bbMaxX -= distFactor(zz);
		bbMinY -= distFactor(zz);
		bbMaxY -= distFactor(zz);
	}
	
	// check if bounding square outside of kinect view
	if(xx < bbMinX)
		bbMinX = xx;
	if(yy < bbMinY)
		bbMinY = yy;

	if(yy > height-bbMaxY)
		bbMaxY = height - yy;
	if(xx > height-bbMaxX)
		bbMaxX = width - xx;

	char handPosBuffer[50];
	int len;
	len=std::sprintf (handPosBuffer, "HAND POS - x: %i y:%i z:%i", xx, yy, zz);
	string s = handPosBuffer;
	ofDrawBitmapString(s, 100, 100, 0);
	/*
	vector<ofPoint> handPoints;
	vector<ofPoint> handRootPoints;
	vector<ofPoint> handEdgePoints; 
	vector<ofPoint> handPalmCandidates;
	*/
	handPoints.clear();
	handRootPoints.clear();
	handEdgePoints.clear();
	handPalmCandidates.clear();

	//handPoints.push_back(ofPoint(0,0,0)); // add point to end of vector
		
	int step = 2;
	minX = ofGetWidth(), maxX = 0, minY = ofGetHeight(), maxY = 0;
	maxZ = 0, minZ = 10000; //?

	int userID = 0; // all users

	for(int y = yy-bbMinY; y < yy+bbMaxY; y += step) {
		for(int x = xx-bbMinX; x < xx+bbMaxX-step; x += step) {

			ofPoint nextPoint = userGen->getWorldCoordinateAt(x+step, y, userID);
			ofPoint pos = userGen->getWorldCoordinateAt(x, y, userID);

			/*
			if(abs(pos.z - nextPoint.z) > 50) {
				if(nextPoint.z != 0 && nextPoint.distance(handTrackedPos) < 150 && nextPoint.distance(handTrackedPos) > 20)  //
					handEdgePoints.push_back(nextPoint);
				else if(pos.z != 0 && pos.distance(handTrackedPos) < 150 && pos.distance(handTrackedPos) > 20) // 
					handEdgePoints.push_back(pos);
			}*/
			
			if (pos.z == 0 || pos.distance(handTrackedPos) > (160 - distFactor(zz))) continue;	// gets rid of background -> still a bit weird if userID > 0...
			//|| abs(pos.z - zz) > 50

			//BBox
			if(pos.x < minX)
				minX = pos.x;
			else if(pos.x > maxX)
				maxX = pos.x;

			if(pos.y < minY)
				minY = pos.y;
			else if(pos.y > maxY)
				maxY = pos.y;

			if(pos.z < minZ)
				minZ = pos.z;
			else if(pos.y > maxZ)
				maxZ = pos.z;

			if (pos.distance(handTrackedPos) <= (160 - distFactor(zz))
			 && pos.distance(handTrackedPos) > (152 - distFactor(zz))) // filter most outer points
				handRootPoints.push_back(pos);
			/*else if(pos.distance(handTrackedPos) < 30 && pos.distance(handTrackedPos) > 0)
				handPalmCandidates.push_back(pos);*/
			/*ofColor color = user_generator->getWorldColorAt(x,y, userID);
			glColor4ub((unsigned char)color.r, (unsigned char)color.g, (unsigned char)color.b, (unsigned char)color.a);
			glVertex3f(pos.x, pos.y, pos.z);
			*/

			//TODO: check for 2 hands tracking - separate hand points for each hand
			else
				handPoints.push_back(pos);
		}
	}
	