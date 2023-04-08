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
	