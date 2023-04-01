
#include "ofxHandModel.h"

#define F1_CLOSED_X	0
#define F2_CLOSED_X	0
#define F3_CLOSED_X	0
#define F4_CLOSED_X	0

#define F1_OPENED_X	25 * PI/180
#define F2_OPENED_X	0
#define F3_OPENED_X	-20 * PI/180
#define F4_OPENED_X	-35 * PI/180

ofxHandModel::ofxHandModel(void)
{
	origin = ofPoint(0, 0, 0);
	//rotation = ofPoint(0, 0, 0);
	scaling = ofPoint(0.3, 0.3, 0.3);
	/*
	t = ofxThumbModel(origin + ofPoint(0, 150, 50));
	f[0]->setLength(140, 80, 60);
	//f[0]->root.origin = origin + ofPoint(0, 50, 115);

	f1 = ofxFingerModel(ofPoint(0, -80+40-60, 75));
	f[1]->setLength(85, 75, 65);
	f[1]->root.angleX = 11;//0.19;
	//f[1]->minAngleX = F1_CLOSED_X;
	//f[1]->maxAngleX = F1_OPENED_X;

	f2 = ofxFingerModel(ofPoint(0, -100+40-60, 25));
	f[2]->setLength(95, 85, 75);
	f[2]->root.angleX = 0;
	//f[2]->minAngleX = F2_CLOSED_X;
	//f[2]->maxAngleX = F2_OPENED_X;

	f3 = ofxFingerModel(ofPoint(0, -90+40-60, -25));
	f[3]->setLength(90, 80, 70);
	f[3]->root.angleX = -10; //-0.17;
	//f[3]->minAngleX = F3_CLOSED_X;
	//f[3]->maxAngleX = F3_OPENED_X;

	f4 = ofxFingerModel(ofPoint(0, -70+40-60, -75));
	f[4]->setLength(70, 60, 50);
	f[4]->root.angleX = -17; //-0.30;
	//f[4]->minAngleX = F4_CLOSED_X;
	//f[4]->maxAngleX = F4_OPENED_X;
	*/
	f[0] = new ofxThumbModel(origin + ofPoint(0, 150, 50));
	f[0]->setLength(130, 70, 50);

	f[1] = new ofxFingerModel(ofPoint(0, -80+40-60, 75));
	f[1]->setLength(75, 65, 55);
	f[1]->root.angleX = 11;

	f[2] = new ofxFingerModel(ofPoint(0, -100+40-60, 25));
	f[2]->setLength(85, 75, 65);
	f[2]->root.angleX = 0;

	f[3] = new ofxFingerModel(ofPoint(0, -90+40-60, -25));
	f[3]->setLength(80, 70, 60);
	f[3]->root.angleX = -10; 

	f[4] = new ofxFingerModel(ofPoint(0, -70+40-60, -75));
	f[4]->setLength(60, 50, 40);
	f[4]->root.angleX = -17;
	
	// Quaternion rotation init
	//this slows down the rotate a little bit
	dampen = .4;
	curRot.makeRotate(90, ofVec3f(0,1,0));

	// init of experimental section with fbos & shaders
	ofFbo::Settings s = ofFbo::Settings();  
	s.width = IMG_DIM;  
	s.height = IMG_DIM;  
	s.useDepth = true;  
	s.useStencil = true;  
	s.depthStencilAsTexture = true;
	meshFbo.allocate(s);  
	meshFbo.setUseTexture(true);

	//meshFbo.allocate(IMG_DIM, IMG_DIM);

	dilateFbo.allocate(IMG_DIM, IMG_DIM);

	cout << "FBO objects supported: " << meshFbo.checkGLSupport() << endl;

	dilateShader.load("shaders/dilate");

	projImg.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE);
	projImg.setImageType(OF_IMAGE_GRAYSCALE);

	projPix.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE);
}


ofxHandModel::~ofxHandModel(void)
{
	for(int i=0; i<NUM_FINGERS; i++) {
		delete f[i];
		f[i] = NULL;
	}
}

void ofxHandModel::update()
{
	//cout << "ofxHandModel origin X: " << origin.x << " Y: " << origin.y << " Z: " << origin.z << endl;
	// update all fingers
	/*f[1]->update();
	f[2]->update();
	f[3]->update();
	f[4]->update();
	f[0]->update();*/
	interpolationTimer.update();

	for(int i=0; i<NUM_FINGERS; i++) {
		f[i]->update();
	}

	// here we should store all new vertices to some mesh or vbo object