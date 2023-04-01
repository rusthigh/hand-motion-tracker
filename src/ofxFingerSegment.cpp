
#include "ofxFingerSegment.h"

ofxFingerSegment::ofxFingerSegment(ofPoint _origin)
{
	length = 100;
	angleZ = 0;//-90; //-PI/2;
	//angleX = 0.6;

	refAngleX = 0;
	refAngleZ = 90;

	origin = _origin;

	/*direction.x = cos(angleZ) * length;
	direction.y = sin(angleZ) * sin(angleX) * length;
	direction.z = cos(angleX) * length;*/

	//rotationMatrix.makeRotationMatrix(angleX * 180/PI, ofVec3f(1, 0, 0), 0, ofVec3f(0,0,0), angleZ * 180/PI, ofVec3f(0, 0, 1));
}

ofxFingerSegment::ofxFingerSegment(void)
{
	length = 100;
	angleZ = 0;//-90; //-PI/2;
	//angleX = 0.6;

	refAngleX = 0;
	refAngleZ = 90;

	origin.x = ofGetWidth()/2;
	origin.y = ofGetHeight()/2;
	origin.z = 0;

	/*direction.x = cos(angleZ) * length;
	direction.y = sin(angleZ) * sin(angleX) * length;
	direction.z = cos(angleX) * length;*/
}

ofxFingerSegment::~ofxFingerSegment(void)
{
}

void ofxFingerSegment::update()
{
	//direction.x = cos(angleZ) * length;
	//direction.y = sin(angleZ) * length;
	//direction.z = 0;

	//rotationMatrix.makeRotationMatrix(angleZ * 180/PI, ofVec3f(0, 0, 1), angleX * 180/PI, ofVec3f(1, 0, 0), 0, ofVec3f(0,1,0));
	//rotationMatrix.makeRotationMatrix(angleZ * 180/PI, ofVec3f(0, 0, 1), 0, ofVec3f(0,1,0), angleX * 180/PI, ofVec3f(1, 0, 0));
	//rotationMatrix.makeRotationMatrix(angleZ * 180/PI, ofVec3f(0, 0, 1));
	
//	rotateByZ.makeRotationMatrix(angleZ * 180/PI, ofVec3f(0, 0, 1)); //working with rad
//	rotateByX.makeRotationMatrix(angleX * 180/PI, ofVec3f(1, 0, 0));

	//FIXED: i forgot what this 90DEG stands for, now i am lost :S
	//UPDATE: -90DEG rotation replaced with reference angle which means startup rotation
	rotateByZ.makeRotationMatrix(angleZ-refAngleZ, ofVec3f(0, 0, 1)); 
	rotateByX.makeRotationMatrix(angleX-refAngleX, ofVec3f(1, 0, 0));

	//rotationMatrix.
	//direction = rotationMatrix.postMult(ofVec3f(-length, 0, 0));
	direction = rotateByZ.postMult(ofVec3f(-length, 0, 0));