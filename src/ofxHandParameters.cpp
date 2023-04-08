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
		//cout << "PARAMS: " << params << " INDEX: " << index << " VALUE: " << valu