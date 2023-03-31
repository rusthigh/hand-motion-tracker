
#include "ofxFingerModel.h"


ofxFingerModel::ofxFingerModel(ofPoint _origin) 
{
	root = ofxFingerSegment(_origin);

	// origin + direction changes
	mid.origin = root.origin + root.direction;
	mid.direction = root.direction;
	top.origin = mid.origin + mid.direction;
	top.direction = mid.direction;

	// x angle changes
	mid.angleX = root.angleX;
	top.angleX = mid.angleX;

	root.length = 100;
	mid.length = 90;
	top.length = 80;

	root.update();
	mid.update();
	top.update();

	angleDiff = 10; // 1 deg per keypress
}

ofxFingerModel::ofxFingerModel(void)
{
	angleDiff = 10; // 1 deg per keypress

	// origin + direction changes
	mid.origin = root.origin + root.direction;
	mid.direction = root.direction;
	top.origin = mid.origin + mid.direction;
	top.direction = mid.direction;

	// x angle changes
	mid.angleX = root.angleX;
	top.angleX = mid.angleX;