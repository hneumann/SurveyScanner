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

	int processFrame(string);

public:
	SurveyScanner(string);

};