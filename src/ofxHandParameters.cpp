#include "ofxHandParameters.h"

/* DEPRECATED:
Parameters::Parameters(void)
{
}


Parameters::~Parameters(void)
{
}
*/
/*
DiscreteLocalParameters::DiscreteLocalParameters(int _params) {
	params = _params;

	//params = params & 0x11111;

	//cout << "INPUT PARAMS: " << params << endl;

	int value = 0;
	int index = 0;

	while(index < 5) {
		value = params & 0x1;
		states[index] = value;
		//cout << "PARAMS: " << params << " INDEX: " << index << " VALUE: " << value << endl;

		params = params >> 1; //params / 2;
		index++;
	}
}
*/

ofxFingerParameters ofxFingerParameters::operator+(const ofxFingerParameters&  other)
{
	ofxFingerParameters temp;

	temp.fz1 = fz1 + other.fz1;
	temp.fz2 = fz2 + other.fz2;
	temp.fz3 = fz3 + other.fz3;
	temp.fz4 = fz4 + other.fz4;

	temp.fx1 = fx1 + other.fx1;
	temp.fx2 = fx2 + other.fx2;
	temp.fx3 = fx3 + other.fx3;
	temp.fx4 = fx4 + other.fx4;
	
	temp.tz = tz + other.tz;
	temp.tx = t