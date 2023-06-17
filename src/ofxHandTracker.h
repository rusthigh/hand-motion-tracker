#pragma once

//#include "ofMain.h"
#include "ofxHandModel.h"
#include "ofxOpenNI.h"
#include "ofxOpenCv.h"
#include "ofxImageMatcher.h"

#define	TINY_IMG_DIM		150
#define FTIP_HIST_SIZE		3	// history array size

#define	HAND_GRABBED			"hand_grabbed"
#define HAND_RELEASED			"hand_released"
#define HAND_PICKED				"hand_picked"

#define MIN_HAND_DEPTH		 500.0
#define MAX_HAND_DEPTH	    2000.0

// tracker for each hand
class ofxHandTracker
{
	public:
		//ofxHandTracker();
		ofxHandTracker(ofxUserGenerator *_userGen, ofxHandGenerator *_handGen, ofxDepthGenerator *_depthGen, int _hIndex);
		//~ofxHandTracker(void);

		void update();
		void draw();

	// getters 
		ofxHandModel*			getHandModel();
		vector<ofPoint> getActiveFingerTips();
		int				getNumFingerTips();
		
		//ofEvent<string> handEvent;	// will notify about grabbing, releasing, etc.

	private:
		//OpenNI Generator references 
		ofxUserGenerator	*userGen;
		ofxHandGenerator	*handGen;
		ofxDepthGenerator	*depthGen; // not needed, remove if useless in future

		// depth image fbo
		ofFbo				depthFbo;

		//hand model
		ofxHandModel				h;
		int					hIndex;

		//Images
		ofImage				blankImg;  //image for clearing (overriding) real image pixels
		//	-> hand image from point cloud
		ofImage				realImg;
		ofxCvGrayscaleImage realImgCV;
		ofxCvGrayscaleImage realImgCV_previous; // not used right now, stores info about hand pose