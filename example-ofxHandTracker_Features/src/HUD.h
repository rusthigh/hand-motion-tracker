#pragma once

#include "ofMain.h"

class HUD
{
public:
	HUD(void);
	HUD(ofPoint _mapPos, ofColor _mapColor, int _w, int _h);
	HUD(ofPoint _mapPos, int _mapWidth, int _mapHeight, float _wPages, float _hPages, bool _lockDim = false);
	HUD(ofPoint _mapPos, int _mapWidth, int _mapHeight, in