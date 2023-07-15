#pragma once

#include "ofMain.h"
#include <string>

class ofxUtilityTimer
{
	public:
        ofxUtilityTimer(void);
		~ofxUtilityTimer(void);
        
        bool                    update();
        void					draw(float _x, float _y);
		void					draw(float _x, float _y, float _w, float _h, ofColor _color);

        void                    start(fl