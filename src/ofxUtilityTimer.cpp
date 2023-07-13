
#include "ofxUtilityTimer.h"


ofxUtilityTimer::ofxUtilityTimer(void)
{
    /*isCounting = false;
    minutes=0; seconds=0; miliseconds=0;
    durationTime=0; step=0;*/
	stop();
}

bool ofxUtilityTimer::update()
{
    elapsedMillis = ofGetElapsedTimeMillis() - startTime;

    if(!isZero())
    {
        step = durationTime - ofGetElapsedTimeMillis();
                
		//miliseconds = (int)step % 1000;
		//seconds = (int) (miliseconds / 1000) % 60 ;
		//minutes = (int) ((int)(miliseconds / (1000*60)) % 60);
        
        /*miliseconds=(float)((int)step%1000)*0.1;
        minutes=step/1000;
        seconds=((int)minutes)%60;
        minutes/=60;*/
                
       // if(minutes<0) minutes=0;
       // if(miliseconds<0) miliseconds=0;

        return true;
    }
	stop();
	//send event here
    return false;
}

bool ofxUtilityTimer::isZero() {
	return !(durationTime > 0 && step > 0 && isCounting);
}

void ofxUtilityTimer::start(float millis)
{
	isCounting = true;

    durationMillis = millis;
    durationTime = ofGetElapsedTimeMillis() + durationMillis;
      
    step = durationMillis;
    startTime = ofGetElapsedTimeMillis();
}

void ofxUtilityTimer::stop()
{