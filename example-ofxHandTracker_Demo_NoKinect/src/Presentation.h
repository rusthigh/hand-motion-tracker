//
//  Presentation.h
//  BSystem
//
//  Created by Blaz on 9/17/13.
//
//

#pragma once

#include <iostream>
#include "ofMain.h"
#include "ofxUtilityTimer.h"

//#define		FULL_HD_W   1920.0
//#define     FULL_HD_H	1080.0

#define		FULL_HD_W   1000.0
#define     FULL_HD_H	800.0

#define TIMER_SLIDE_DURATION	60000
#define TIMER_TOTAL_DURATION	600000

class Presentation
{

 public:
    Presentation();
    Presentation(string _assetsFilename, string _menusFilename, string _vizualType);
    ~Presentation();
    
    void setup();
    void update();
    void draw();
    
    void        windowResized(int w, int h);

    void        enable();
    void        disable();

    // mouse events (for timeout reset)
    void        mousePr