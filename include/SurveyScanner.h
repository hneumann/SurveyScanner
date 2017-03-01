#pragma once

#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>

using namespace cv;

class SurveyScanner
{
private:
	vector<string> surveys;
	vector<Point> checkedPositions;

	string preprocessMode;

	Mat reference;
	Mat raw;
	Mat frame;
	Mat original;

	int preProcessFrame(string);
	int preProcessFrameWithReference(string);
	int process();

	Size rSize;

	int skalaStart;
	int skalaEnd;

	int numChecks;
	int mode; //0: semi, 1: automatic



public:
	SurveyScanner(string);
	void mouseHandler(int event, int x, int y);
};