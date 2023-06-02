
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
	
	handCentroid = getCentroid(handPoints);

//	ofPoint prevHandRootCentroid = handRootCentroid;
	handRootCentroid = getCentroid(handRootPoints);

	ofPoint maxDistCentroidPoint = ofPoint(handCentroid);
	float maxDistCentroid = 0;

	ofPoint maxDistTrackedPoint = ofPoint(handTrackedPos);
	float maxDistTracked = 0;

	ofPoint maxDistRootPoint = ofPoint(handRootCentroid);
	float maxDistRoot = 0;

	// calculate approx. oritentation by calcing max distance from centroid
	for(std::vector<ofPoint>::iterator it = handPoints.begin(); it != handPoints.end(); ++it) {
		/* std::cout << *it; ... */
		ofPoint p = *it;
		if(p.distance(handCentroid) > maxDistCentroid) {
			maxDistCentroid = p.distance(handCentroid);
			maxDistCentroidPoint = ofPoint(p);
		}
		if(p.distance(handTrackedPos) > maxDistTracked) {
			maxDistTracked = p.distance(handCentroid);
			maxDistTrackedPoint = ofPoint(p);
		}
		if(p.distance(handRootCentroid) > maxDistRoot) {
			maxDistRoot = p.distance(handRootCentroid);
			maxDistRootPoint = ofPoint(p);
		}
	}

	// prevent jumping rootCentroid to 0,0,0, need better solution
	if(handRootCentroid.distance(ofPoint(0,0,0)) < 10)
		handRootCentroid = handCentroid;

	/*bool usingCentroid = false;
	
	ofPoint estimatedPalmCenter = palmCenter;
	if(estimatedPalmCenter.distance(ofPoint::zero()) < 10) {
		estimatedPalmCenter = handCentroid;
		usingCentroid = true;
	}*/

	int i = 0;
	for(std::vector<ofPoint>::iterator it = handPoints.begin(); it != handPoints.end(); ++it) {
		ofPoint pos = *it;
		i++;
		//if(usingCentroid) {
			if (i%2 == 0 || pos.distance(handCentroid) > (maxDistCentroid * 0.60)) continue;
		/*}
		else
			if (pos.distance(estimatedPalmCenter) > 60) continue;
		*/
		handPalmCandidates.push_back(pos);
	}
}

float ofxHandTracker::distFactor(float zDist) {
	float zz = zDist;
	return (zz) * 1/30;
}

ofPoint ofxHandTracker::getPalmCenter() {
	/* PALM CENTER CALCULATION - from: http://blog.candescent.ch/2011/04/center-of-palm-hand-tracking.html
	1. foreach x-th candidate 
	2. calculate the smallest distance to any point in the contour
	3. then take the candidate with the largest minimum distance 
	*/
	float minDistance = 1000000;
	float minMaxDistance = 0;

	ofPoint minCandidate;
	ofPoint minMaxCandidate;

	//for(std::vector<ofPoint>::iterator canIt = handPalmCandidates.begin(); canIt != handPalmCandidates.end(); ++canIt) {
	for(std::vector<ofPoint>::iterator canIt = imgPalmCandidates.begin(); canIt != imgPalmCandidates.end(); ++canIt) {
	
		ofPoint canPos = *canIt;
		minDistance = 10000000;

		//for(std::vector<ofPoint>::iterator edgeIt = handEdgePoints.begin(); edgeIt != handEdgePoints.end(); ++edgeIt) {
		for(std::vector<ofPoint>::iterator edgeIt = blobPoints.begin(); edgeIt != blobPoints.end(); ++edgeIt) {
		
			ofPoint edgePos = *edgeIt;
			
			float currentDistance = edgePos.distance(canPos);
			if (currentDistance < minDistance && currentDistance > 5){ //
				minDistance = currentDistance;
				minCandidate = canPos;
			}
		}

		if(minDistance > minMaxDistance){
			minMaxDistance = minDistance;
			minMaxCandidate = minCandidate;
		}
	}

	//ofNoFill();
	//ofCircle(minMaxCandidate, minMaxDistance);
	//ofFill();

	palmRadius = minMaxDistance;
	return minMaxCandidate;
}

void ofxHandTracker::update() {
	/*
	depthFbo.begin();
	int tx = handGen->getHand(hIndex)->projectPos.x;
	int ty = handGen->getHand(hIndex)->projectPos.y;
	ofRect(0,0,IMG_DIM,IMG_DIM);
	depthGen->draw(-tx + IMG_DIM/2, -ty + IMG_DIM/2);
	depthFbo.end();
	*/
	ofPoint prevActiveHandPos = activeHandPos;

	
		// new way of image clearing - any faster?
		realImg.setFromPixels(blankImg.getPixelsRef());
	/*
		for (int i=0; i<IMG_DIM; i++) {    
			for (int j=0; j<IMG_DIM; j++) {    
				realImg.setColor(i, j, ofColor::black); 
				//modelImg.setColor(i, j, ofColor::black);
				//colorImg.setColor(i, j, ofColor::black);
			}    
		} 
	*/
	bool handDetected = false;

	if(handGen->getNumTrackedHands() > 0) { // else use handGen
		int xx = handGen->getHand(hIndex)->projectPos.x;
		int yy = handGen->getHand(hIndex)->projectPos.y;
		int zz = handGen->getHand(hIndex)->projectPos.z;
		int raw = handGen->getHand(hIndex)->rawPos.Z;

		activeHandPos = ofPoint(xx,yy,zz);

		if(activeHandPos.distance(ofPoint::zero()) > 10) {
			h.origin.x = activeHandPos.x;
			h.origin.y = activeHandPos.y;

			handDetected = true;
		}
	}
	else if (userGen->getNumberOfTrackedUsers() > 0) { // if skeleton available use skeleton data
  												// here we can determine left or right hand?
		for (int i=0; i < userGen->getNumberOfTrackedUsers(); i++) {
			if(i == 0) {
				ofxTrackedUser* currentUser = userGen->getTrackedUser(i+1);
					
				ofxLimb closerArm;
				if (currentUser->left_lower_arm.position[1].Z < currentUser->right_lower_arm.position[1].Z) {
					closerArm = currentUser->left_lower_arm;
					//if (h.scaling.z < 0)
					//	h.scaling.z = abs(h.scaling.z);
				}
				else {
					closerArm = currentUser->right_lower_arm;
					//if (h.scaling.z > 0)
					//	h.scaling.z = -abs(h.scaling.z);
				}

				int xx = closerArm.position[1].X;
				int yy = closerArm.position[1].Y;
				int zz = closerArm.position[1].Z;
				int raw = handGen->getHand(hIndex)->rawPos.Z;

				activeHandPos = ofPoint(xx,yy,zz);

				if(activeHandPos.distance(ofPoint::zero()) > 10) {
					//h.origin.x = h.origin.x + (handTrackedPos.x - h.origin.x);
					//h.origin.y = handTrackedPos.y;

					h.origin = activeHandPos;
					h.origin.z = 0;

					handDetected = true;
				}
			}
		}

	}
	

	if(handDetected) {
		fetchHandPointCloud(activeHandPos);	

		for(std::vector<ofPoint>::iterator it = handPoints.begin(); it != handPoints.end(); ++it) {
			ofPoint pos = *it;
			/*realImg.setColor((pos.x - palmCenter.x)*(IMG_DIM/300.0) + IMG_DIM/2, // too much shaking
							 (pos.y - palmCenter.y)*(IMG_DIM/300.0) + IMG_DIM/2, 
							 ofColor((-pos.z +  palmCenter.z)+128, 255));*/
			
			float tX = (pos.x - handCentroid.x)*(IMG_DIM/300.0) + IMG_DIM/2;
			float tY = (pos.y - handCentroid.y)*(IMG_DIM/300.0) + IMG_DIM/2;
			if (tX >= 0 && tX < IMG_DIM && tY >= 0 && tY < IMG_DIM) {
				// if crosses border causes 0xC0000005: Access violation reading location
				int col = 255 - (((pos.z - (handCentroid.z))/0.5)+128);
				if (col >= 255) col = 255;
				if (col <= 0)	col = 0;

				realImg.setColor((pos.x - handCentroid.x)*(IMG_DIM/300.0) + IMG_DIM/2, 
								 (pos.y - handCentroid.y)*(IMG_DIM/300.0) + IMG_DIM/2, 
								 //ofColor(((-pos.z + handCentroid.z)+128), 255));
								 ofColor(col, 255));
			}
		}

		//ofPixels depthPixels;
		//depthFbo.readToPixels(depthPixels);
		//colorImg.setFromPixels(depthPixels);

		imgPalmCandidates.clear();
		for(std::vector<ofPoint>::iterator it = handPalmCandidates.begin(); it != handPalmCandidates.end(); ++it) {
			ofPoint pos = *it;

			imgPalmCandidates.push_back(ofPoint((pos.x - handCentroid.x)*(IMG_DIM/300.0) + IMG_DIM/2,
												(pos.y - handCentroid.y)*(IMG_DIM/300.0) + IMG_DIM/2, 0));

		}

		ofPoint prevPalmCenter = palmCenter;
		palmCenter = getPalmCenter();

		if(palmCenter.distance(ofPoint::zero()) < 10)
			palmCenter = prevPalmCenter;
		//if(palmCenter.distance(prevPalmCenter) > 50 && prevPalmCenter.distance(ofPoint(0,0,0)) > 100) // bad solution
		//	palmCenter = prevPalmCenter;
		else {
			palmCenter = prevPalmCenter + ((palmCenter - prevPalmCenter)*0.6f); // we can smooth & interpolate to new center
		}
		//cout << "PALM CENTER: " << palmCenter << endl;
		realImg.setColor(palmCenter.x, palmCenter.y, 255);


		// calculate average orientation
		/*ofPoint orientationVector;
		for(std::vector<ofPoint>::iterator it = handEdgePoints.begin(); it != handEdgePoints.end(); ++it) {
			ofPoint pos = *it;
			orientationVector += (pos - handRootCentroid);
		}
		ofPoint pos = handRootCentroid + orientationVector;
		
		glBegin(GL_LINES);
		glColor4ub(255, 128, 0, 0);
		glVertex3f(pos.x, pos.y, pos.z);
		glVertex3f(handRootCentroid.x, handRootCentroid.y, handRootCentroid.z);
		glEnd();
		*/
			
		ofPoint dirDown = ofPoint(handCentroid.x, handCentroid.y+50, 0);
		ofPoint dirRot = ofPoint(handRootCentroid.x, handRootCentroid.y, 0);
		
		//ofPoint downVector = dirDown - handCentroid;
		//ofPoint rotVector = dirRot - handCentroid;
		
		ofPoint downVector = ofPoint(0, -50, 0);
		ofPoint rotVectorZ = ofPoint(handRootCentroid.x - handCentroid.x,
									 handRootCentroid.y - handCentroid.y, 
									 0);
		ofPoint rotVectorX = ofPoint(0,
									 handRootCentroid.y - handCentroid.y, 
									 handRootCentroid.z - handCentroid.z);

		
		float prevRollAngle = rollAngle;
		rollAngle = angleOfVectors(downVector, rotVectorZ);

		if (abs(rollAngle - prevRollAngle) < 180)
			rollAngle = prevRollAngle + ((rollAngle - prevRollAngle)*0.5f); // smoothing


		//cout << " hand angle: " << angle << endl;
		/*
		double x = (handCentroid.x - handRootCentroid.x);
		double y = (handCentroid.y - handRootCentroid.y);
		
		double angleInRadiansXY = std::atan2(y, x);
		double angleInDegreesXY = (angleInRadiansXY / PI) * 180.0 + 90;
		*/

		// pre-rotate, so hand is facing us
		h.curRot = ofQuaternion(90, ofVec3f(0,1,0));

		// check if hand facing sensor directly and rotate accordingly
		// TODO: make smooth transition
		if(rotVectorZ == ofPoint::zero()) {
			//cout << rotVectorZ <<endl;
			rollAngle = 0;
			h.curRot *= ofQuaternion((-90), ofVec3f(1,0,0));
		}
		else {
			float angle = angleOfVectors(downVector, rotVectorX, false);
			//cout << "xangle: " << angle << endl;
			h.curRot *= ofQuaternion((180+angle), ofVec3f(1,0,0));
		}

		//h.curRot *= ofQuaternion((angleInDegreesXY), ofVec3f(0,0,1));
		h.curRot *= ofQuaternion((rollAngle), ofVec3f(0,0,1));
		


		/*
		//x = (maxZ - minZ);
		x = (handCentroid.z - handRootCentroid.z);
		
		double angleInRadiansZY = std::atan2(y, x);
		double angleInDegreesZY = (angleInRadiansZY * 180/PI) + 90;
		
		//h.curRot *= ofQuaternion((angleInDegreesZY), ofVec3f(1,0,0));
		*/
		realImg.update();

		// maybe scaling like this?
		//realImg.setAnchorPercent(0.5, 0.5);
		//realImg.resize();
		//realImg.crop();

		h.update();
		
		generateModelProjection();
		
		analyzeContours();

		// here call contourAnalysis method when implemented

		//modelImgCV.dilate();
		/*modelImgCV.dilate();
		modelImgCV.dilate();
		modelImgCV.dilate();
		modelImgCV.dilate();*/
	//}


	//for(int itY=0; itY<3; itY++) 
	///for(int itX=0; itX<3; itX++) {
		//int startX = palmCenter.x + (itX-1)*15; //(int)(IMG_DIM/2); //
		//int startY = palmCenter.y + (itY-1)*15; //(int)(IMG_DIM/2); //

		// here we try with searching for peaks on image
	/*
	int numberOfPoints = 8;
	float angle = 360.0f/numberOfPoints;
    
    for (int itX=0; itX<numberOfPoints; itX++) {

		int startX = palmCenter.x + 20 * cosf((angle*itX)*PI/180);
		int startY = palmCenter.y + 20 * sinf((angle*itX)*PI/180);
		//int max = modelImg.getColor(i,j).getBrightness();
		int maxVal = 0;
		int maxX = 0, maxY = 0;
		int counter = 0;

		int neighbours[3][3];

		while(counter < 20) {
		
			counter++;

			//cout << "3x3 part from img: \n >--------<" << endl;
			for(int i=0; i<3; i++) {
				for(int j=0; j<3; j++) {
					neighbours[i][j] = realImg.getColor(startX + (i-1), startY + (j-1)).getBrightness();
					//cout << " " << neighbours[i][j] << " ";

					if(i==1 && j==1)
						neighbours[i][j]= -1;
				}
				cout << endl;
			}

			bool newMaxFound = false; 

			for(int i=0; i<3; i++) {
				for(int j=0; j<3; j++) {
					if(neighbours[i][j] >= maxVal) {
						maxVal = neighbours[i][j];
						maxX = (i-1);
						maxY = (j-1);

						newMaxFound = true;
					}
				}
			}

			if(!newMaxFound)
				break;

			//neighbours[i][j] = modelImg.getColor(x + (i-1),y + (j-1)).getBrightness();

			startX += maxX;
			startY += maxY;

			maxX = 0, maxY = 0;

			if(startX < 0 || startX >= IMG_DIM)
				break;
			if(startY < 0 || startY >= IMG_DIM)
				break;

			realImg.setColor(startX, startY, ofColor::black); 
		}
	}
	realImg.update();
	*/
	
		/*handMask.setFromPixels(depthGen.getDepthPixels(zz-100, zz+100),
							   depthGen.getWidth(), 
							   depthGen.getHeight(), 
							   OF_IMAGE_GRAYSCALE);*/

		//handMask.setFromPixels(userGen.getUserPixels(), userGen.getWidth(), userGen.getHeight(), OF_IMAGE_GRAYSCALE);
			
		/*handMask.setFromPixels(depthGen.getDepthPixels(zz-120, zz+120),
						depthGen.getWidth(), 
						depthGen.getHeight(), 
						OF_IMAGE_GRAYSCALE);*/

		//if(thresh != 0)
		
		// temporary calling convex hull methods
		/*if(handEdgePoints.size() > 0) {
		//if(filteredMaxHandPoints.size() > 5) {
			//ofPoint pivot = ofPoint(maxDistRootPoint);
			ofPoint pivot = ofPoint(xx-bbMinX, yy+bbMaxY, 0);

			//pivot = ofPoint(minX, maxY, handCentroid.z);
			vector<ofPoint> convexHull = getConvexHull(handEdgePoints, pivot);

			//cout << "EDGE SIZE: " << handEdgePoints.size() << "\nHULL SIZE: " << convexHull.size() << endl;

			drawOfPointVector(convexHull);
			drawOfPointVectorFromCenter(convexHull, handCentroid);
		}
		*/


		// calc another angle
		/*double y2 = (handCentroid.y - handRootCentroid.y);
		double z = (handCentroid.z - handRootCentroid.z);
		double angleInRadiansYZ = std::atan2(y2, z);
	    double angleInDegreesYZ = (angleInRadiansYZ / PI) * 180.0 + 90;
		h.curRot *= ofQuaternion((angleInDegreesYZ), ofVec3f(1,0,0));
		*/

		//float handAngle = atan2(maxDistCentroidPoint.x-maxDistOppositePoint.x, maxDistCentroidPoint.y-maxDistOppositePoint.y);

		//float centroidDistance2D = sqrt(pow(handCentroid.x - handRootCentroid.x,2) + pow(handCentroid.y - handRootCentroid.y,2));

	}
}

void ofxHandTracker::analyzeContours() {
	realImgCV_previous.setFromPixels(realImgCV.getPixelsRef());

	realImgCV.setFromPixels(realImg.getPixelsRef());
	realImgCV.dilate();
	realImgCV.erode();

	realImgCV_previous.absDiff(realImgCV);

	tinyModelImg.scaleIntoMe(modelImgCV);
	tinyHandImg.scaleIntoMe(realImgCV);

	realImgCvContour.findContours(realImgCV,0,IMG_DIM*IMG_DIM,1, false, false);

	int prevFingerTipsCounter = fingerTipsCounter;
	fingerTipsCounter = -1;

	if(realImgCvContour.blobs.size() > 0) {
	//if(realImgCvContour.nBlobs > 0) {

		ofxCvBlob handBlob = realImgCvContour.blobs[0];
		//blobPoints.clear();
		blobPoints = handBlob.pts;

		int size = blobPoints.size();
		
		vector<float> angles;
		vector<ofPoint>	fingerTips;

		int step = 12;
		float minAngle = 360;