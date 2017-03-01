#pragma once

#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>

using namespace cv;

class SurveyScanner
{
private:
	vector<string> surveys;
	Mat reference;
	Mat raw;
	Mat original;

	int processFrame(string);
	int processFrameWithReference(string);
	vector<Point> checkedPositions;

	double skalaStart; 
	double skalaEnd;

	int numChecks;
	int mode; //0: semi, 1: automatic



public:
	SurveyScanner(string);
	void mouseHandler(int event, int x, int y);
};