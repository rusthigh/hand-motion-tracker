
#include "HUD.h"


HUD::HUD(void)
{
	mapWidth = ofGetWidth()/2;
	mapHeight = ofGetHeight()/2;

	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	realWidth = ofGetWidth() * 3;
	realHeight = ofGetHeight() * 3;

	mapPos = ofPoint(10, 10, 0);
	initMapPos = mapPos;
	locPos = mapPos;

	mapColor = ofColor(64, 128, 96, 128);
	locColor = ofColor(255, 64, 64);

	lockDim = false;
}

HUD::HUD(ofPoint _mapPos)
{
	mapWidth = ofGetWidth()/2;
	mapHeight = ofGetHeight()/2;
	
	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	realWidth = ofGetWidth() * 3;
	realHeight = ofGetHeight() * 3;

	mapPos = _mapPos;
	initMapPos = mapPos;
	locPos = mapPos;

	mapColor = ofColor(64, 128, 96, 128);
	locColor = ofColor(255, 64, 64);

	lockDim = false;
}

HUD::HUD(ofPoint _mapPos, ofColor _mapColor, int _mapWidth, int _mapHeight)
{
	mapColor = _mapColor;
	locColor = ofColor(255, 176, 32);

	mapWidth = _mapWidth;
	mapHeight = _mapHeight;
	
	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	realWidth = ofGetWidth() * 3;
	realHeight = ofGetHeight() * 3;

	mapPos = _mapPos;
	initMapPos = mapPos;
	locPos = _mapPos;

	lockDim = false;
}

HUD::HUD(ofPoint _mapPos, int _mapWidth, int _mapHeight, float _wPages, float _hPages, bool _lockDim) {
	// init size of map
	mapWidth = _mapWidth;
	mapHeight = _mapHeight;
	
	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	// num of pages from params
	hPages = _hPages;
	wPages = _wPages;

	// actual area available to user
	realWidth = ofGetWidth() * wPages;
	realHeight = ofGetHeight() * hPages;

	mapPos = _mapPos;
	initMapPos = mapPos;
	locPos = _mapPos;

	lockDim = _lockDim;
}

// TODO: update dimensions when screen resized (based on realW/H or pagesW/H -> depends how we need hud to be resized)
HUD::HUD(ofPoint _mapPos, int _mapWidth, int _mapHeight, int _realWidth, int _realHeight, bool _lockDim) {
	mapWidth = _mapWidth;
	mapHeight = _mapHeight;

	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	realWidth = (float)(_realWidth);
	realHeight = (float)(_realHeight);

	wPages = (float)(realWidth)/(float)(ofGetWidth());
	hPages = (float)(realHeight)/(float)(ofGetHeight());

	// fix when HUD is created with greater resolution than existing screen resolution 
	// (prevents shaking which happens beacuse of autocorrecting & border checking)
	if(wPages < 1)
		realWidth = ofGetWidth();
	if(hPages < 1)
		realHeight = ofGetHeight();