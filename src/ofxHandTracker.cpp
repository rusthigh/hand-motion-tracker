
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
		int minIndex = -1;

		fingerTipsCounter = 0;

		for(int i=step; i < (size+step); i++) { //*=step*/
			ofPoint prevPos = blobPoints[(i-step)];
			ofPoint curPos = blobPoints[(i)%size];
			ofPoint nextPos = blobPoints[(i+(step))%size];

			ofPoint prevVector = curPos - prevPos;
			ofPoint nextVector = curPos - nextPos;

			float kCosine = prevVector.dot(nextVector)/(prevVector.length()*nextVector.length());
			float angle = acosf(kCosine)*180/PI; //*180/PI
			//cout << "index : " << i << " angle: " << angle << endl;

			float normalZ = prevVector.crossed(nextVector).z; // for filtering peaks -> fingertips

			// here we search for minimum angles which define fingertips and gaps between them
			// also check if normalZ is greater or equal zero for fingertips
			if (angle < 60.0 && normalZ >= 0)
			{
				if (minAngle >= angle) {
					minIndex = i;
					minAngle = angle;
				}
			}
			else if (minIndex != -1){

					fingerTipsCounter++;
					fingerTips.push_back(ofPoint(blobPoints[(minIndex)%size].x, blobPoints[(minIndex)%size].y, 0));
					
					//fingerTips.at(fingerTipsCounter++) = ofPoint(blobPoints[(minIndex)%size].x, blobPoints[(minIndex)%size].y, 0);

					minAngle = 360;
					minIndex = -1;
			}

			angles.push_back(angle);
		}

		ofPoint rotVectorZ = ofPoint(handRootCentroid.x - handCentroid.x,
									 handRootCentroid.y - handCentroid.y, 
									 0);

		int prevCounter = fTipHistory[((fTipLastInd-1)%FTIP_HIST_SIZE)];

		fTipHistory[(fTipLastInd++)%FTIP_HIST_SIZE] = fingerTipsCounter;

		float avgFingerTips = 0;

		for (int i=0; i<FTIP_HIST_SIZE; i++) {
			avgFingerTips += fTipHistory[i];
		}
		avgFingerTips /= FTIP_HIST_SIZE;
		
		/*bool fistFormed = false;
		if(fingerTipsCounter == 0) {
			fistFormed = true;
			for (int i=0; i<FTIP_HIST_SIZE; i++) {
				if(fTipHistory[i] != 0)
					fistFormed = false;
			}

			//ofNotifyEvent(grabEvent, "grabbed");

			if(!fistFormed) {
				/*
				int index = (fTipLastInd-1)%FTIP_HIST_SIZE;
				while(fTipHistory[index] == 0) {
					index--;
					index = (index)%FTIP_HIST_SIZE;
				}
				fingerTipsCounter = fTipHistory[((index-1)%FTIP_HIST_SIZE)];
				*
				fingerTipsCounter = prevCounter;
			}
		}*/

		//fingerTipsCounter = floor(avgFingerTips+0.5); //round to nearest int
		//fingerTipsCounter = (int)(avgFingerTips+0.5);
		int avgFingers = (int)(avgFingerTips+0.5);
		fingerTipsCounter = avgFingers;
		/*
		stringstream ss;
		ss << "F TIP HIST: ";
		for (int i=0; i<FTIP_HIST_SIZE; i++) {
			ss << fTipHistory[i] << " ";
		}
		ss << endl;
		cout << ss.str() << "AVG FTIPS: " << avgFingerTips <<  endl;
		*/
		
		activeFingerTips.clear();

		for(int i=0; i<fingerTips.size(); i++) {
			ofPoint fTip = fingerTips[i];
			ofPoint tempTip = fingerTips[(i+1)%fingerTips.size()];

			float angle = angleOfVectors(rotVectorZ, fTip - palmCenter);

			int xx = handGen->getHand(hIndex)->projectPos.x;
			int yy = handGen->getHand(hIndex)->projectPos.y;
			ofPoint handTrackedPos = ofPoint(xx,yy,0);

			activeFingerTips.push_back((fTip - palmCenter) + handTrackedPos);
		}

		// TODO: consider creating set of parameters which define only x angles (lets say 4 different parameter objects)
		// then for each case in checking for best parameters add one more for loop and each time add x angle param to current param
		// that way we can check for multiple finger poses (by x angle)
		// also we should check for which fingers we have to apply angles (not to all, just to these which are straight)
		// PARTIALLY DONE: distinguishing between fist and aligned fingers position is working
		const int X_PARAMS_SIZE = 6;
		ofxFingerParameters xParams[X_PARAMS_SIZE];

		xParams[0] = ofxFingerParameters(11, 0, -10, -17, 0); //default hand setup
		xParams[1] = ofxFingerParameters(9, 0, -8, -14, 0); 
		xParams[2] = ofxFingerParameters(7, 0, -6, -9, 0);
		xParams[3] = ofxFingerParameters(4, 0, -4, -4, 0);
		xParams[4] = ofxFingerParameters(1, 0, -2, 1, 0);
		xParams[5] = ofxFingerParameters(-3, 0, -1, 5, 0);    // fingers aligned tight by x angle

		//int p[2];
		//p[0] = (31);
		//p[1] = (0);

		//cout << "prev_curr img diff: " << getImageMatching(realImgCV_previous) << endl;
		//if (getImageMatching(realImgCV_previous) > 5) // 10?
		//	findParamsOptimum(p, 2);

		//TODO: here update hand model logic which is handled by different rules (palm radius, fingerTipsCounter, ...)
		if(fingerTipsCounter == 0) {
		//if(avgFingers == 0) {
		//if(fistFormed) {
			//here check if hand completely closed or just closed fingers
			ofxFingerParameters zeroTips[2];
			zeroTips[0] = ofxFingerParameters(31) + ofxFingerParameters(1, 0, -1, -2, -30); // just fingers
			zeroTips[1] = ofxFingerParameters(0);				// fist formed
			findParamsOptimum(zeroTips, 2);
		}
		else if (fingerTipsCounter >= 5) {
		//else if (avgFingers >= 5) {
			ofxFingerParameters p = ofxFingerParameters(31);
		//	p = p + xParams[0];

			//const int X_PARAMS_5_SIZE = 6;
			//ofxFingerParameters xParams5[X_PARAMS_5_SIZE];

			xParams[0] = ofxFingerParameters(11, 0, -10, -17, 0); //default hand setup
			xParams[1] = ofxFingerParameters(9, 0, -8, -14, -5); 
			xParams[2] = ofxFingerParameters(7, 0, -6, -9, -10);
			xParams[3] = ofxFingerParameters(4, 0, -4, -4, -15);
			xParams[4] = ofxFingerParameters(1, 0, -2, 1, -20);
			xParams[5] = ofxFingerParameters(0, 0, 0, 2, -25);    // fingers aligned tight by x angle

			int params[] = {31};
			int size = 1;

			//findParamsOptimum(params, size, xParams, X_PARAMS_SIZE); // not working properly
			//findParamsOptimum(params, size);
			h.restoreFrom(p, false);
			//h.interpolate(p);
		}
		else if(fingerTipsCounter == 1) {
		//else if (avgFingers == 1) {
			//int params1[] = {1, 2, 4, 8, 16};
			//int size = 5;
			// with no ring finger cause it's difficult to form that shape - 8,
			//int params1[] = {1, 2, /*4,*/ 16};
			//int size = 3;

			int params1[] = {2};
			int size = 1;
			//findParamsOptimum(params1, size, xParams, X_PARAMS_SIZE);
			findParamsOptimum(params1, size);
		}
		else if(fingerTipsCounter == 2) {
		//else if (avgFingers == 2) {
			// 12 is index & middle which is hard to form together
			int params2[] = {3, 6, 17, 18}; 
			int size = 4;
			//findParamsOptimum(params2, size, xParams, X_PARAMS_SIZE);
			findParamsOptimum(params2, size);
		}
		else if(fingerTipsCounter == 3) {
		//else if (avgFingers == 3) {
			int params3[] = {7, 11, 14, 19, 22, 28};
			int size = 6;
			findParamsOptimum(params3, size);
		}
		else if(fingerTipsCounter == 4) {
		//else if (avgFingers == 4) {
			int params4[] = {15, 23, 27, 29, 30};
			int size = 5;
			findParamsOptimum(params4, size);
		}
	}
}

float ofxHandTracker::angleOfVectors(ofPoint _downVector, ofPoint _rotVector, bool _absolute) {
	_downVector.normalize();
	_rotVector.normalize();

	float cosine = _downVector.dot(_rotVector); ///(downVector.length()*rotVector.length());
	float angle = acosf(cosine)*180/PI; //*180/PI
	
	if (_absolute) {
		float normalZ = _downVector.crossed(_rotVector).z; // for filtering peaks -> fingertips
		if(normalZ < 0)
			angle *= -1;
		angle += 180;
	}

	return angle;
}

void ofxHandTracker::draw() {

	int userID = 0;

	int width = userGen->getWidth();
	int height = userGen->getHeight();
		
	//int xx = handGen->getHand(handIndex)->projectPos.x;
	//int yy = handGen->getHand(handIndex)->projectPos.y;
	//int zz = handGen->getHand(handIndex)->projectPos.z;
	int thresh = handGen->getHand(hIndex)->rawPos.Z;

	glPushMatrix();
	glTranslatef(-width/2, -height/2, -thresh*3);
	ofScale(2,2,2);
	
	glBegin(GL_POINTS);

	// drawing points
	for(std::vector<ofPoint>::iterator it = handPoints.begin(); it != handPoints.end(); ++it) {
		ofPoint pos = *it;
		ofColor color = userGen->getWorldColorAt(pos.x,pos.y, userID);
		glColor4ub((unsigned char)color.r, (unsigned char)color.g, (unsigned char)color.b, (unsigned char)color.a);
		glVertex3f(pos.x, pos.y, pos.z);
	}

	glColor4ub(127, 255, 0, 127);
	for(std::vector<ofPoint>::iterator it = handRootPoints.begin(); it != handRootPoints.end(); ++it) {
		// std::cout << *it; 
		ofPoint pos = *it;
		glVertex3f(pos.x, pos.y, pos.z);
	}

	//for(std::vector<ofPoint>::iterator it = handEdgePoints.begin(); it != handEdgePoints.end(); ++it) {
	//	ofPoint pos = *it;
	//	glColor4ub(255, 0, 255, 0);
	//	glVertex3f(pos.x, pos.y, pos.z);
	//}

	//for(std::vector<ofPoint>::iterator it = handPalmCandidates.begin(); it != handPalmCandidates.end(); ++it) {
	//	ofPoint pos = *it;
	//	glColor4ub(255, 100, 100, 0);
	//	glVertex3f(pos.x, pos.y, pos.z);
	//}

	glEnd();
	
	//draw BBox
	/*glBegin(GL_LINES);
	glColor3f(0.5f, 1.0f, 0.5f);
	glVertex3f(minX, minY, handCentroid.z);
	glVertex3f(maxX, minY, handCentroid.z);
	glVertex3f(minX, maxY, handCentroid.z);
	glVertex3f(maxX, maxY, handCentroid.z);

	glVertex3f(minX, minY, handCentroid.z);
	glVertex3f(minX, maxY, handCentroid.z);
	glVertex3f(maxX, minY, handCentroid.z);
	glVertex3f(maxX, maxY, handCentroid.z);
	glEnd();*/

	// drawing orientation
	
	glBegin(GL_LINES);

	glColor3f(1.0f, 0.0f, 1.0f);
	// first (half) line of orientation
	//glVertex3f(handCentroid.x, handCentroid.y, handCentroid.z);
	/*glVertex3f(maxDistCentroidPoint.x, maxDistCentroidPoint.y, maxDistCentroidPoint.z);
	// second half (line in opposite direction)
	//glVertex3f(handCentroid.x, handCentroid.y, handCentroid.z);
	glVertex3f(maxDistOppositePoint.x, maxDistOppositePoint.y, maxDistOppositePoint.z);


	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(handTrackedPos.x, handTrackedPos.y, handTrackedPos.z);
	glVertex3f(maxDistTrackedPoint.x, maxDistTrackedPoint.y, maxDistTrackedPoint.z);*/
	//TODO: make these vars members of class so we can draw them
	glVertex3f(handCentroid.x, handCentroid.y, handCentroid.z);
	glVertex3f(handRootCentroid.x, handRootCentroid.y, handRootCentroid.z);
	
	glVertex3f(handCentroid.x, handCentroid.y, handCentroid.z);
	glVertex3f(handCentroid.x, handCentroid.y+50, handCentroid.z);
	

	//glVertex3f(palmCenter.x, palmCenter.y, palmCenter.z);

	/*glColor3f(0.0f, 1.0f, 1.0f);
	glVertex3f(handRootCentroid.x, handRootCentroid.y, handRootCentroid.z);
	glVertex3f(maxDistRootPoint.x, maxDistRootPoint.y, maxDistRootPoint.z);
	*/
	//glColor3f(1.0f, 1.0f, 0.0f);
	//glVertex3f(handCentroid.x, handCentroid.y, handCentroid.z);
	// palm center vars
	//glVertex3f(minMaxCandidate.x, minMaxCandidate.y, minMaxCandidate.z);
	//glVertex3f(minCandidate.x, minCandidate.y, minCandidate.z);

	glEnd();

	glPopMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);

	ofDrawBitmapString("HAND MODEL:", 20, 460);
	ofPushStyle();
	//ofSetColor(255, 0, 0);
	modelImg.draw(0, 480, 200, 200);
	ofPopStyle();

	ofDrawBitmapString("DILATED\nHAND MODEL:", 220, 450);
	modelImgCV.draw(200, 480, 200, 200);

	//h.draw();
	//h.getProjection().draw(200, 680, 200, 200);

	ofDrawBitmapString("HAND \nPOINT CLOUD:", 420, 450);

	ofPushStyle();
	//ofSetColor(0, 255, 0);
	realImg.draw(400, 480, 200, 200);
	ofPopStyle();

	ofDrawBitmapString("DILATED HAND \nPOINT CLOUD:", 620, 450);
	realImgCV.draw(600, 480, 200, 200);

	ofDrawBitmapString("PREV FRAME \n ABS DIFF:", 620, 680);
	realImgCV_previous.draw(600, 680, 200, 200);

	realImgCvContour.draw(1000, 480, 200, 200);
	ofDrawBitmapString("CV Contour:", 1060, 460);

	// just drawing part of contour analysis
	if(realImgCvContour.blobs.size() > 0) {
	//if(realImgCvContour.nBlobs > 0) {

		ofxCvBlob handBlob = realImgCvContour.blobs[0];

		int size = blobPoints.size();
		
		glPushMatrix();
		glTranslatef(1500, 0, 0);
		ofScale(2,2,1);
		ofNoFill();
		ofSetColor(255,255,255);
		ofCircle(palmCenter, palmRadius);
		ofFill();
		glPopMatrix();
		
		vector<float> angles;
		vector<ofPoint>	fingerTips;

		int step = 12;
		float minAngle = 360;
		int minIndex = -1;

		for(int i=step; i < (size+step); i++) { //*=step*/
			ofPoint prevPos = blobPoints[(i-step)];
			ofPoint curPos = blobPoints[(i)%size];
			ofPoint nextPos = blobPoints[(i+(step))%size];

			ofPoint prevVector = curPos - prevPos;
			ofPoint nextVector = curPos - nextPos;

			float kCosine = prevVector.dot(nextVector)/(prevVector.length()*nextVector.length());
			float angle = acosf(kCosine)*180/PI; //*180/PI
			//cout << "index : " << i << " angle: " << angle << endl;

			float normalZ = prevVector.crossed(nextVector).z; // for filtering peaks -> fingertips

			ofPushMatrix();
			glTranslatef(1250, 0, 0);
			ofScale(2,2,1);
			ofSetColor(255-(angle*255.0/360.0), 0, angle*255.0/360.0);
			ofRect(blobPoints[(i)%size].x, blobPoints[(i)%size].y, 2,2);

			glTranslatef(-25, 0, 0);
			ofScale(0.5,0.5,1);
			glBegin(GL_LINES);
			glColor4ub(255-(angle*255.0/360.0), 0, angle*255.0/360.0, 255); // hand contour angle plotting
			glVertex3f(i+100, 500, 0);
			glVertex3f(i+100, 500 - angle, 0);
			glEnd();

			ofPopMatrix();

			glPushMatrix();
			glTranslatef(1500, 0, 0);
			ofScale(2,2,1);
			// here we search for minimum angles which define fingertips and gaps between them
			// also check if normalZ is greater or equal zero for fingertips
			if (angle < 60.0 && normalZ >= 0)
			{
				if (minAngle >= angle) {
					minIndex = i;
					minAngle = angle;
				}
			}
			else if (minIndex != -1){
					ofSetColor(255,0,0);
					ofRect(blobPoints[(minIndex)%size].x, blobPoints[(minIndex)%size].y, 3, 3);

					glBegin(GL_LINES);
			
						glColor4ub(128, 255, 128, 255);
						glVertex3f(blobPoints[(minIndex)%size].x, blobPoints[(minIndex)%size].y, 0);
						glVertex3f(palmCenter.x, palmCenter.y, 0);
			
					glEnd();

					minAngle = 360;
					minIndex = -1;
			}
			
			glBegin(GL_LINES);
			
			glColor4ub(255, 255, i, 255);
			glVertex3f(blobPoints[(i)%size].x, blobPoints[(i)%size].y, 0);
			glVertex3f(blobPoints[(i+1)%size].x, blobPoints[(i+1)%size].y, 0);
			glEnd();
			
			glPopMatrix();
		}

		glPushMatrix();
		glTranslatef(1325, 500, 0);
		ofScale(2,2,1);
		for(int i=0; i<fingerTipsCounter; i++) {
			ofSetColor(255 - i*255/fingerTipsCounter, i*255/fingerTipsCounter, 0);
			ofRect(i*50, 0, 50, 50);
		}
		glPopMatrix();

		for(int i=0; i<fingerTips.size(); i++) {
			ofPoint fTip = fingerTips[i];
			ofPoint tempTip = fingerTips[(i+1)%fingerTips.size()];

			ofSetColor(0, 255, 0);
			ofRect(fTip.x, fTip.y, 10, 10);
			
			ofSetColor(50*(i+1), 0, 0);
			ofLine(fTip, tempTip);
		}
	}
	h.draw();

	// comparison between legacy & shader projection generation
	/*generateModelProjection(true);
	modelImgCV.draw(10, 900, 300, 300);
	generateModelProjection(false);
	modelImgCV.draw(310, 900, 300, 300);
	*/

	//float matching = getImageMatching(realImgCV, modelImgCV, diffImg);
	/*float matching = 0;// getImageMatching(tinyHandImg, tinyModelImg, tinyDiffImg);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	char matchingBuffer[50];
	int len;
	len=sprintf (matchingBuffer, "HAND/MODEL MATCH: %f", matching);

	string s = matchingBuffer;
	ofDrawBitmapString(s, 800, 460, 0);

	diffImg.draw(800, 480, 200, 200);
	tinyDiffImg.draw(800, 480, 200, 200);
	*/
	ofPushStyle();
		ofSetColor(255, 65, 170);
		ofDrawBitmapString("EDGE SET SIZE: " + ofToString(handEdgePoints.size()) + 
					   "\n% PALM SET SIZE: " + ofToString(handPalmCandidates.size()) + 
					   "\n% HAND SET SIZE: " + ofToString(handPoints.size()), 20, 180);
	ofPopStyle();
	

	ofPushStyle();
		ofSetColor(255, 65, 170);
		ofDrawBitmapString("RAW Z: " + ofToString(getHandDepth(activeHandPos.z, false)) + 
						   "\n% Z: " + ofToString(getHandDepth(activeHandPos.z)), 20, 150);
	ofPopStyle();
	
	ofPushStyle();
		ofSetColor(255, 65, 170);
		ofDrawBitmapString("FINGERTIPS: " + ofToString(fingerTipsCounter), 20, 120);
	ofPopStyle();
}

float ofxHandTracker::getHandDepth(float _rawZ, bool _normalized, float _minZ, float _maxZ) {
	float raw = _rawZ;

	if (_normalized) {		
		float percentage = (float)(raw - _minZ)/(float)(_maxZ - _minZ); // od 0 do 1500/1500
		percentage = ofClamp(percentage, 0, 1);

		return percentage;
	}
	return raw;
}

void ofxHandTracker::findParamsOptimum(int _params[], int _size) {
	ofxFingerParameters bestParams = ofxFingerParameters(0);
	float			bestMatching = 0;
	//float			bestMatching = IMG_DIM*IMG_DIM;
	for(int i=0; i<_size; i++) {
		ofxFingerParameters curParams = ofxFingerParameters(_params[i]);
		h.restoreFrom(curParams);

		tinyModelImg.set(0);
		generateModelProjection();
		tinyModelImg.scaleIntoMe(modelImgCV);
		tinyHandImg.scaleIntoMe(realImgCV);

		//float matching = getImageMatching(tinyHandImg, tinyModelImg, tinyDiffImg);
		//float matching = shaderMatcher.matchImages(modelImg, realImg);
		float matching = getImageMatching(modelImg, shaderMatcher.imagesAbsDiff(modelImg, realImg));

		if(matching > bestMatching) 
		//if(matching < bestMatching && matching > 0.01)
		{
			bestMatching = matching;
			bestParams = curParams;
		}
	}

	ofSetColor(255,255,255);
	ofDrawBitmapString("best_match: " + ofToString(bestMatching), 30, 30);

	h.restoreFrom(bestParams);
}

void ofxHandTracker::findParamsOptimum(ofxFingerParameters _params[], int _size) {
	ofxFingerParameters bestParams = ofxFingerParameters(0);
	float			bestMatching = 0;
	//float			bestMatching = IMG_DIM*IMG_DIM;
	for(int i=0; i<_size; i++) {
		ofxFingerParameters curParams = _params[i];
		h.restoreFrom(curParams);

		tinyModelImg.set(0);
		generateModelProjection();
		tinyModelImg.scaleIntoMe(modelImgCV);
		tinyHandImg.scaleIntoMe(realImgCV);

		//float matching = getImageMatching(tinyHandImg, tinyModelImg, tinyDiffImg);
		//float matching = shaderMatcher.matchImages(modelImg, realImg);
		float matching = getImageMatching(modelImg, shaderMatcher.imagesAbsDiff(modelImg, realImg));

		if(matching > bestMatching) 
		//if(matching < bestMatching && matching > 0.01)
		{
			bestMatching = matching;
			bestParams = curParams;
		}
	}

	ofSetColor(255,255,255);
	ofDrawBitmapString("best_match: " + ofToString(bestMatching), 30, 30);

	h.restoreFrom(bestParams);
}

void ofxHandTracker::findParamsOptimum(int _paramsZ[], int _sizeZ, ofxFingerParameters _paramsX[], int _sizeX) {
	ofxFingerParameters bestParams = ofxFingerParameters(0);
	float			bestMatching = 0;
	//float			bestMatching = IMG_DIM*IMG_DIM;
	for(int i=0; i<_sizeZ; i++) {
		ofxFingerParameters curParams = ofxFingerParameters(_paramsZ[i]);
		for(int j=0; j<_sizeX; j++) {
			ofxFingerParameters merged = curParams + _paramsX[j];

			h.restoreFrom(merged, true);

			generateModelProjection();
			tinyModelImg.scaleIntoMe(modelImgCV);
			tinyHandImg.scaleIntoMe(realImgCV);

			//float matching = getImageMatching(tinyHandImg, tinyModelImg, tinyDiffImg);
			//float matching = shaderMatcher.matchImages(modelImg, realImg);
			float matching = getImageMatching(modelImg, shaderMatcher.imagesAbsDiff(modelImg, realImg));

			tinyModelImg.set(0);

			if(matching > bestMatching) 
			//if(matching < bestMatching && matching > 0.01)
			{
				bestMatching = matching;
				bestParams = merged;
			}
		}
	}

	ofSetColor(255,255,255);
	ofDrawBitmapString("best_match: " + ofToString(bestMatching), 30, 30);

	h.restoreFrom(bestParams, true);
}

/*ofxHandTracker::~ofxHandTracker(void)
{
	//free(userGen);
	//free(handGen);
	//userGen = NULL;
	//handGen = NULL;
}*/


//------------------------- helper methods -------------------------------------
// here maybe rather than void we should return generated image directly?
void ofxHandTracker::generateModelProjection(bool _useLegacy) {
	if (_useLegacy) {
		
		for (int i=0; i<IMG_DIM; i++) {    
			for (int j=0; j<IMG_DIM; j++) {    
				modelImg.setColor(i, j, ofColor::black);    // ofColor(ofRandom(255), ofRandom(255), ofRandom(255))
			}    
		}
		
			float dOx = palmCenter.x;// IMG_DIM/2; // draw offset
			float dOy = palmCenter.y;//IMG_DIM/2; // draw offset

			for(int i=0; i<=4; i++){
				vector<ofPoint> joints1 = h.getFingerWorldCoord(i);
				drawLine(&modelImg, -(h.origin.x - joints1[0].x)*0.5 + dOx, -(h.origin.y - joints1[0].y)*0.5 + dOy, -(h.origin.z - joints1[0].z),
					-(h.origin.x - joints1[1].x)*0.5 + dOx, -(h.origin.y - joints1[1].y)*0.5 + dOy, -(h.origin.z - joints1[1].z));
				drawLine(&modelImg, -(h.origin.x - joints1[1].x)*0.5 + dOx, -(h.origin.y - joints1[1].y)*0.5 + dOy, -(h.origin.z - joints1[1].z),
					-(h.origin.x - joints1[2].x)*0.5 + dOx, -(h.origin.y - joints1[2].y)*0.5 + dOy, -(h.origin.z - joints1[2].z));
				drawLine(&modelImg, -(h.origin.x - joints1[2].x)*0.5 + dOx, -(h.origin.y - joints1[2].y)*0.5 + dOy, -(h.origin.z - joints1[2].z), 
					-(h.origin.x - joints1[3].x)*0.5 + dOx, -(h.origin.y - joints1[3].y)*0.5 + dOy, -(h.origin.z - joints1[3].z));
		
				//drawLine(-(h.origin.x - joints1[0].x)*0.5 + 64, -(h.origin.y - joints1[0].y)*0.5 + 64, 0, 64, 64, 0);
			}

			vector<ofPoint> filling = h.getFillWorldCoord();

			//for(std::vector<ofPoint>::iterator it = filling.begin(); it != filling.end(); ++it) {
			for(int i=0; i<filling.size(); i += 2) {
				/* std::cout << *it; ... */
					ofPoint p1 = filling[i];
					ofPoint p2 = filling[i+1];

					drawLine(&modelImg, -(h.origin.x - p1.x)*0.5 + dOx, -(h.origin.y+0.2 - p1.y)*0.5 + dOy, -(h.origin.z - p1.z), 
							 -(h.origin.x - p2.x)*0.5 + dOx, -(h.origin.y+0.2 - p2.y)*0.5 + dOy, -(h.origin.z - p2.z));
			}

			modelImg.update(); 

			modelImgCV.setFromPixels(modelImg.getPixelsRef());
			// todo set dilate based on scaling
			modelImgCV.dilate();
			modelImgCV.dilate();
			modelImgCV.dilate();
			
	}
	else {
		ofPoint backupScaling = h.scaling;
		// rescale model, so it fits better to real hand depth image
		// 0.6 downscales the actual scaling range (from 0 - 1 to 0 - 0.6),
		// - 0.15 adds some startup scaling, so model is not underscaled 
		h.scaling -= ((getHandDepth(activeHandPos.z)*0.6) - 0.15); 

		//cout << "PALM CENTER: " << palmCenter << endl;
		palmCenter.z = 0;
		modelImg = h.getProjection(palmCenter, 6*(1-getHandDepth(activeHandPos.z)));
		//modelImg = h.getProjection();
		modelImg.setImageType(OF_IMAGE_GRAYSCALE);
		modelImgCV.setFromPixels(modelImg);

		h.scaling = backupScaling;
	}
		//modelImgCV.dilate();
		//modelImgCV.erode();
}
/*
ofImage ofxHandTracker::generateModelProjection(ofxHandModel h) {

	ofImage modelImg;

	for (int i=0; i<IMG_DIM; i++) {    
		for (int j=0; j<IMG_DIM; j++) {    
			modelImg.setColor(i, j, ofColor::black);    // ofColor(ofRandom(255), ofRandom(255), ofRandom(255))
		}    
	}
		
		float dOx = palmCenter.x;// IMG_DIM/2; // draw offset
		float dOy = palmCenter.y;//IMG_DIM/2; // draw offset

		for(int i=1; i<=5; i++){
			vector<ofPoint> joints1 = h.getFingerWorldCoord(i);
			drawLine(&modelImg, -(h.origin.x - joints1[0].x)*0.5 + dOx, -(h.origin.y - joints1[0].y)*0.5 + dOy, -(h.origin.z - joints1[0].z),
				-(h.origin.x - joints1[1].x)*0.5 + dOx, -(h.origin.y - joints1[1].y)*0.5 + dOy, -(h.origin.z - joints1[1].z));
			drawLine(&modelImg, -(h.origin.x - joints1[1].x)*0.5 + dOx, -(h.origin.y - joints1[1].y)*0.5 + dOy, -(h.origin.z - joints1[1].z),
				-(h.origin.x - joints1[2].x)*0.5 + dOx, -(h.origin.y - joints1[2].y)*0.5 + dOy, -(h.origin.z - joints1[2].z));
			drawLine(&modelImg, -(h.origin.x - joints1[2].x)*0.5 + dOx, -(h.origin.y - joints1[2].y)*0.5 + dOy, -(h.origin.z - joints1[2].z), 
				-(h.origin.x - joints1[3].x)*0.5 + dOx, -(h.origin.y - joints1[3].y)*0.5 + dOy, -(h.origin.z - joints1[3].z));
		
			//drawLine(-(h.origin.x - joints1[0].x)*0.5 + 64, -(h.origin.y - joints1[0].y)*0.5 + 64, 0, 64, 64, 0);
		}

		vector<ofPoint> filling = h.getFillWorldCoord();

		//for(std::vector<ofPoint>::iterator it = filling.begin(); it != filling.end(); ++it) {
		for(int i=0; i<filling.size(); i += 2) {
				ofPoint p1 = filling[i];
				ofPoint p2 = filling[i+1];

				drawLine(&modelImg, -(h.origin.x - p1.x)*0.5 + dOx, -(h.origin.y+0.2 - p1.y)*0.5 + dOy, -(h.origin.z - p1.z), 
						 -(h.origin.x - p2.x)*0.5 + dOx, -(h.origin.y+0.2 - p2.y)*0.5 + dOy, -(h.origin.z - p2.z));
		}

		modelImg.update(); 

		modelImgCV.setFromPixels(modelImg.getPixelsRef());
		// todo set dilate based on scaling
		modelImgCV.dilate();
		modelImgCV.dilate();
		modelImgCV.dilate();

		//modelImgCV.dilate();
		//modelImgCV.erode();
}*/

float ofxHandTracker::getImageMatching(ofxCvGrayscaleImage &realImage, 
									ofxCvGrayscaleImage &modelImage,  
									ofxCvGrayscaleImage &differenceImage) {
					
	//ofPixels realPixels = realImage.getPixelsRef();
	//realPixels[0]

	unsigned char *real = realImage.getPixels();
	unsigned char *model = modelImage.getPixels();

	int w = realImage.getWidth();
	int h = realImage.getHeight();

	float allDiff = 0;
	float handSum = 0;
	float modelSum = 0;

	for (int i=0; i<w*h; i++) {
		handSum += real[i];
		real[i] =  abs(model[i] - real[i]);
		allDiff += real[i];

		//draw dilated hand model as point cloud
		//if(model[i] != 0) {
		//	int x = i%(int)(IMG_DIM);
		//	int y = i/(int)(IMG_DIM);
		//	glBegin(GL_POINTS);
		//	glColor3d(model[i], 0, 0);
		//	glVertex3f((x*2) + 100, (y*2)+ 100, 255 - model[i]);
		//	glEnd();
		//}
	}
	
	differenceImage.setFromPixels(real, w, h);
	//differenceImage.erode();

	float matching = (handSum - allDiff)/handSum;
	matching = matching * 10.0/6.0;
	return matching;
}

float ofxHandTracker::getImageMatching(ofxCvGrayscaleImage &differenceImage) {

	unsigned char *diff = differenceImage.getPixels();

	int w = differenceImage.getWidth();
	int h = differenceImage.getHeight();

	float allDiff = 0;

	for (int i=0; i<w*h; i++) {
		allDiff += diff[i];
	}
	
	float matching = allDiff/(w*h);
	//matching = matching * 10.0/6.0;
	return matching;
}


float ofxHandTracker::getImageMatching(ofImage &realImage,   
									ofImage &diffImage) {
					
	ofPixels realPixels = realImage.getPixelsRef();
	ofPixels diffPixels = diffImage.getPixelsRef();

	int w = realImage.getWidth();
	int h = realImage.getHeight();

	float allDiff = 0;
	float handSum = 0;
	float modelSum = 0;

	for (int i=0; i<w; i++) {
		for (int j=0; j<h; j++) {
			handSum += realPixels.getColor(i,j).getBrightness();
			allDiff += diffPixels.getColor(i,j).getBrightness();
		}
	}

	float matching = (handSum - allDiff)/handSum;
	matching = matching * 10.0/6.0;
	return matching;
}
/*
float ofxHandTracker::getImageMatching(ofxCvGrayscaleImage &realImage,   
									ofxCvGrayscaleImage &diffImage) {
					
	ofPixels realPixels = realImage.getPixelsRef();
	ofPixels diffPixels = diffImage.getPixelsRef();

	int w = realImage.getWidth();
	int h = realImage.getHeight();

	float allDiff = 0;
	float handSum = 0;
	float modelSum = 0;

	for (int i=0; i<w; i++) {
		for (int j=0; j<h; j++) {
			handSum += realPixels.getColor(i,j).getBrightness();
			allDiff += diffPixels.getColor(i,j).getBrightness();
		}
	}

	float matching = (handSum - allDiff)/handSum;
	matching = matching * 10.0/6.0;
	return matching;
}*/

ofPoint ofxHandTracker::getCentroid(vector<ofPoint> &points){
		float centroidX = 0, centroidY = 0, centroidZ = 0;

		// calculating of centroid
		for(std::vector<ofPoint>::iterator it = points.begin(); it != points.end(); ++it) {
			/* std::cout << *it; ... */
			ofPoint p = *it;
			centroidX += p.x;
			centroidY += p.y;
			centroidZ += p.z;
		}

		if(points.size() > 0) {
			centroidX /= points.size();
			centroidY /= points.size();
			centroidZ /= points.size();
		}

		ofPoint centroid = ofPoint(centroidX, centroidY, centroidZ);
		return centroid;
}


//Bitmap/Bresenham's line algorithm - source: http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
void ofxHandTracker::drawLine(ofImage *img, int x0, int y0, int z0, int x1, int y1, int z1) {

	z0 += 120;
	z1 += 120;
	int dz = (z1-z0)/20;
	/*if(z0 > z1){
		int temp = z0;
		z0 = z1;
		z1 = temp;
	}*/

	//cout << "z0: " << z0 << " z1: " << z1 << endl;

	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;
 
	
	int startX = x0;
	/*float stepZ;
	if(dx != 0)
		stepZ = dz/dx;
	else if(dy != 0)
		stepZ = dz/dy;
	else
		stepZ = 1;*/
	//int step = 2;
	for(;;){
		//if(step%2 == 0)
			img->setColor(x0, y0, ofColor(z0, 255));
		//step++;
		
		z0 += dz;
		
		//	img.setColor(x0, y0, ofColor::white);
		
		/*img.setColor(x0+1, y0+1, ofColor::white);
		img.setColor(x0+1, y0-1, ofColor::white);
		img.setColor(x0-1, y0+1, ofColor::white);
		img.setColor(x0-1, y0-1, ofColor::white);*/
		//rasterizedModel.setROI(x0, y0, 2, 2);
		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 > -dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

// section with methods which provide useful data from tracker

vector<ofPoint> ofxHandTracker::getActiveFingerTips() {
	return activeFingerTips;
}

ofxHandModel* ofxHandTracker::getHandModel() {
	return &h;
}

int	ofxHandTracker::getNumFingerTips() {
	return fingerTipsCounter;
}

/*
void ofxHandTracker::drawLine(ofImage *img, int x1, int y1, int z1, int x2, int y2, int z2)
{
	z1 += 120;
	z2 += 120;

	int dz = (z2-z1)/20;

        // Bresenham's line algorithm
	const bool steep = (abs(y2 - y1) > abs(x2 - x1));
	if(steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}
 
	if(x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
 
	const float dx = x2 - x1;
	const float dy = abs(y2 - y1);
 
	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = (int)y1;
 
	const int maxX = (int)x2;
 
	for(int x=(int)x1; x<maxX; x++)
	{
		if(steep)
                {
					img->setColor(y, x, ofColor(z1, 255));
                }
		else
                {
					img->setColor(x, y, ofColor(z1, 255));
                }
 
		//z1 += dz;

                error -= dy;
	        if(error < 0)
	        {
		        y += ystep;
		        error += dx;
	        }
	}
}*/