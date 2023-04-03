
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
	// to be able to use it in shaders, for better drawing, dilatation etc.
	modelMesh.clear();
	palmMesh.clear();

	for(int i=0; i<NUM_FINGERS; i++) {
		
		modelMesh.addVertex(f[i]->root.origin);
		modelMesh.addVertex(f[i]->mid.origin);
		modelMesh.addVertex(f[i]->mid.origin);
		modelMesh.addVertex(f[i]->top.origin);
		modelMesh.addVertex(f[i]->top.origin);
		modelMesh.addVertex(f[i]->fingerTip);

		// for proper depth coloring
		modelMesh.addColor(getPointColor(f[i]->root.origin));
		modelMesh.addColor(getPointColor(f[i]->mid.origin));
		modelMesh.addColor(getPointColor(f[i]->mid.origin));
		modelMesh.addColor(getPointColor(f[i]->top.origin));
		modelMesh.addColor(getPointColor(f[i]->top.origin));
		modelMesh.addColor(getPointColor(f[i]->fingerTip));
	}



	// setup vertices & depth colors for faces of palm (TRIANGLE_FAN style)
	//--------------------------------------------------------------------------------------------------
	ofPoint f0r = f[0]->root.origin;
	ofPoint f0m = f[0]->mid.origin; 
	ofPoint f1r = f[1]->root.origin;

	ofPoint f2r = f[2]->root.origin;
	ofPoint f3r = f[3]->root.origin;
	ofPoint f4r = f[4]->root.origin;
	
	// custom points for palm
	ofPoint f4c = ofPoint(f[4]->root.origin.x, f[0]->root.origin.y - 30, f[4]->root.origin.z);
	ofPoint f3c = ofPoint(f[3]->root.origin.x, f[0]->root.origin.y, f[3]->root.origin.z);
	ofPoint f0c = ofPoint(f[3]->root.origin.x, f[0]->root.origin.y, f[0]->root.origin.z);

	palmMesh.addVertex(f0r);
	palmMesh.addVertex(f0m);
	palmMesh.addVertex(f1r);

	palmMesh.addColor(getPointColor(f0r));
	palmMesh.addColor(getPointColor(f0m));
	palmMesh.addColor(getPointColor(f1r));

	palmMesh.addVertex(f2r);
	palmMesh.addVertex(f3r);
	palmMesh.addVertex(f4r);

	palmMesh.addColor(getPointColor(f2r));
	palmMesh.addColor(getPointColor(f3r));
	palmMesh.addColor(getPointColor(f4r));

	palmMesh.addVertex(f4c);
	palmMesh.addVertex(f3c);
	palmMesh.addVertex(f0c);

	palmMesh.addColor(getPointColor(f4c));
	palmMesh.addColor(getPointColor(f3c));
	palmMesh.addColor(getPointColor(f0c));
	//--------------------------------------------------------------------------------------------------
}

ofFloatColor ofxHandModel::getPointColor(ofPoint p) {
	float lim = 45.0f;
	ofPoint pc = getWorldCoord(p, origin);
	ofFloatColor fc = ofFloatColor(((pc.z - (origin.z - 30))/lim)/2.0);
	return fc;
}

void ofxHandModel::drawMesh() {
	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();

	//glTranslatef(origin.x, origin.y, origin.z);
	glTranslatef(IMG_DIM/2, IMG_DIM/2, origin.z);
	
	//Extract the rotation from the current rotation
    ofVec3f axis;  
    float angle;  
    curRot.getRotate(angle, axis);  
	
	glRotatef(angle, axis.x, axis.y, axis.z); //apply the quaternion's rotation
	glScalef(scaling.x, scaling.y, scaling.z);
	glScalef(0.5f, 0.5f, 0.5f);

	palmMesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
	palmMesh.drawFaces();

	modelMesh.setMode(OF_PRIMITIVE_LINES);
	modelMesh.draw();

	glPopMatrix();
}

void ofxHandModel::draw() 
{
	/*
	//ofQuaternion constructor: angle, ofVec3f axis
	ofQuaternion qr (rotation.z, ofVec3f(1,0,0));	// quat roll.
	ofQuaternion qp (rotation.x, ofVec3f(0,0,1));	// quat pitch.
	ofQuaternion qh (rotation.y, ofVec3f(0,1,0));	// quat heading or yaw.
	//ofQuaternion qt;					// quat total.

	// The order IS IMPORTANf[0]-> Apply roll first, then pitch, then heading.
	curRot = qr * qp * qh;
	*/
	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();

	glTranslatef(origin.x, origin.y, origin.z);

	/*
	//if(rotation.x != 0)
		glRotatef(rotation.x, 1, 0, 0);
	//if(rotation.y != 0)

	// y and z axis are changed while we rotate around x, so rotations below wont rotate locally -> instead we need to rotate around new (changed) y and z !!!
	// upper line is not totally correct - we need to calculate local axis of the hand each time and rotate around them
	// actually transformations are stacked so rotation around z comes first -> that means first rotation is local, others that are following are not
		glRotatef(rotation.y, 0, 1, 0);
		//glRotatef(rotation.y, sin(rotation.z * PI/180), cos(rotation.z * PI/180), 0);
	//if(rotation.z != 0)
		glRotatef(rotation.z, 0, 0, 1);
		
		glRotatef(90, 0, 1, 0);
		*/
	
	//Extract the rotation from the current rotation
    ofVec3f axis;  
    float angle;  
    curRot.getRotate(angle, axis);  
	
	glRotatef(angle, axis.x, axis.y, axis.z); //apply the quaternion's rotation
	glScalef(scaling.x, scaling.y, scaling.z);
	//glScalef(-scaling.x, scaling.y, scaling.z); // right handed
	
    ofSetColor(ofColor::white);
	ofSphere(ofPoint(0,0,0), 15);
	
	for(int i=0; i<NUM_FINGERS; i++) {
		f[i]->draw();
	}


	ofSetColor(ofColor::green);
	//ofLine(f[0]->root.origin, ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, -f[0]->root.origin.z));
	//ofLine(f[0]->root.origin, ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0));
		
	ofLine(ofPoint(f[0]->mid.origin.x, f[0]->mid.origin.y, f[0]->mid.origin.z),
		   f[1]->root.origin);

	ofLine(f[3]->root.origin,
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[3]->root.origin.z));
	ofLine(f[2]->root.origin,
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[2]->root.origin.z));

	ofLine(f[1]->root.origin,
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[1]->root.origin.z));
	ofLine(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[1]->root.origin.z), 
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0));
	ofLine(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0), 
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[4]->root.origin.z));
	ofLine(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[4]->root.origin.z),
		   f[4]->root.origin);

	// draw hand local coord. system
	ofSetLineWidth(2);
	ofSetColor(ofColor::red);
	ofLine(ofPoint(0,0,0), ofPoint(100,0,0));
	ofSetColor(ofColor::green);
	ofLine(ofPoint(0,0,0), ofPoint(0,100,0));
	ofSetColor(ofColor::blue);
	ofLine(ofPoint(0,0,0), ofPoint(0,0,100));
	ofSetColor(ofColor::white);
	glPopMatrix();
}

void ofxHandModel::drawProjection() 
{
	glMatrixMode(GL_MODELVIEW);	
	glPushMatrix();
	glTranslatef(IMG_DIM/2, IMG_DIM/2, 0);
	
	//Extract the rotation from the current rotation
    ofVec3f axis;  
    float angle;  
    curRot.getRotate(angle, axis);  
	glRotatef(angle, axis.x, axis.y, axis.z); //apply the quaternion's rotation
	glScalef(scaling.x, scaling.y, scaling.z);
	glScalef(0.5f, 0.5f, 0.5f);
	
	// right handed
	//glScalef(-scaling.x, scaling.y, scaling.z);
	
    //ofSetColor(ofColor::white);
	ofSphere(ofPoint(0,0,0), 15);
	//ofSetLineWidth(1);	
	//ofSphere(origin, 15);

	//glLineWidth(8.0f);
	
	for(int i=0; i<NUM_FINGERS; i++) {
		drawFingerProjection(*f[i]);
	}