#include "SurveyScanner.h"

using namespace std;

int main(int argc, char *argv[]) { 

	if (argc != 2) {
		std::cout << "USAGE: SurveyScanner.exe config.yaml" << std::endl;
		return 0;
	}
	SurveyScanner scanner = SurveyScanner(string(argv[1]));

}

SurveyScanner::SurveyScanner(string surveyConfig){
	FileStorage fs(surveyConfig, FileStorage::READ);
	surveys = vector<string>();
	string reference = fs["ReferenceImage"];
	cout << "Reference Image: " << endl << "\t" << reference << endl;
	skalaStart = (int)fs["SkalaStartPx"];
	cout << "Skala start Pixel: " << endl << "\t" << skalaStart << endl;
	skalaEnd = (int)fs["SkalaEndPx"];
	cout << "Skala end Pixel: " << endl << "\t" << skalaEnd << endl;
	int rWidth = (int)fs["ResizeWidth"];
	cout << "ResizeWidth: " << endl << "\t" << rWidth << endl;
	int rHeight = (int)fs["ResizeHeight"];
	cout << "ResizeHeight: " << endl << "\t" << rHeight << endl;
	rSize = Size(rWidth, rHeight);
	numChecks = (int)fs["NumberOfItems"];
	cout << "Number Of Items: " << endl << "\t" << numChecks << endl;
	string mode = fs["Mode"];
	if (mode == "semi"){
		this->mode = 0;
		cout << "Mode semi" << endl;
	}
	if (mode == "automatic"){
		this->mode = 1;
		cout << "Mode automatic" << endl;
	}
	preprocessMode = fs["PrepocessMode"];
	cout << "PrepocessMode" << preprocessMode  << endl;

	Mat raw = imread(reference);
	resize(raw, raw, rSize);
	cvtColor(raw, this->reference, CV_BGR2GRAY);
	//this->reference = this->reference < 250;
	adaptiveThreshold(~this->reference, this->reference, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);
	
	FileNode surveysNode = fs["Surveys"];
	FileNodeIterator it = surveysNode.begin(), it_end = surveysNode.end();
	int idx = 0;
	cout << "Surveys: " << endl;
	for (; it != it_end; ++it, idx++){
		this->surveys.push_back((string)*it);
		cout << "\t" << (string)*it << endl;
	}
	for (string survey : surveys){
		if (preprocessMode == "ReferenceImage")
			preProcessFrameWithReference(survey);
		else if (preprocessMode == "xDetection")
			preProcessFrame(survey);
		process();
	}
}


//This is not used/correct yet.  I tried to implement a check detection by removing the skala (= horizontal and vertical lines) from the image.
int SurveyScanner::preProcessFrame(string path){
	namedWindow("binary", CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED | CV_WINDOW_NORMAL);
	//namedWindow("horizontal", CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED | CV_WINDOW_NORMAL);
	//namedWindow("vertical", CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED | CV_WINDOW_NORMAL);
	std::size_t found = path.find_last_of("/\\");
	cout << endl << "Processing frame " << path.substr(found, path.length() - found) << endl;
	raw = imread(path);
	resize(raw, raw, rSize);
	original = raw.clone();
	cvtColor(raw, frame, CV_BGR2GRAY);
	adaptiveThreshold(~frame, frame, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

	erode(frame, frame, Mat::ones(Size(2,2), CV_8U), Point(-1, -1));
	//dilate(frame, frame, Mat::ones(Size(3,3), CV_8U), Point(-1, -1), 1);

	imshow("binary", frame);
	waitKey();
	Mat kernel = (Mat_<char>(5,5) << 
		 0,0,0,0,0,
		 0,0,0,0,0,
		 -2,-1,6,-1,-2,
		 0,0,0,0,0,
		 0,0,0,0,0);
	Mat kernel2 = (Mat_<char>(5, 5) <<
		0, 0, -2, 0, 0,
		0, 0, -1, 0, 0,
		0, 0, 6, 0, 0,
		0, 0, -1, 0, 0,
		0, 0, -2, 0, 0);

	Mat kernel3 = (Mat_<char>(5, 5) <<
		0, -1, -2, -1, 0,
		1, 0, -1, 0, 1,
		-1, -1, 0, -1, -1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1);
	Mat kernel4 = (Mat_<char>(5, 5) <<
		0, 1, 1, 1, 1,
		-2, 0, 1, 1, 1,
		-2, -2, 0, 1, 1,
		-2, 0, 1, 1, 1,
		0, 1, 1, 1, 1);



	/// Apply filter
	filter2D(frame, frame, -1, kernel3, Point(-1, -1), 0, BORDER_DEFAULT);
	filter2D(frame, frame, -1, kernel, Point(-1, -1), 0, BORDER_DEFAULT);


	dilate(frame, frame, Mat::ones(Size(4,4), CV_8U), Point(-1, -1), 1);
	erode(frame, frame, Mat::ones(Size(4,4), CV_8U), Point(-1, -1));
	//dilate(frame, frame, Mat::ones(Size(3,3), CV_8U), Point(-1, -1), 1);
	//frame = vertical + horizontal;


	Mat verticalStructure = getStructuringElement(MORPH_RECT, Size(8,2));
	// Apply morphology operations
	erode(frame, frame, verticalStructure, Point(-1, -1));
	dilate(frame, frame, verticalStructure, Point(-1, -1));

	Mat horStructure = getStructuringElement(MORPH_RECT, Size(2,8));
	// Apply morphology operations
	erode(frame, frame, horStructure, Point(-1, -1));
	dilate(frame, frame, horStructure, Point(-1, -1));
	imshow("binary", frame);
	//waitKey();

	return -1;
}

void SurveyScanner::mouseHandler(int event, int x, int y){
	if (event == EVENT_LBUTTONUP)
	{
		double result = round((static_cast<double>(x)-skalaStart) * 20.0 / (skalaEnd - skalaStart));
		cout << "Ticked number: " << result << endl;
		checkedPositions.push_back(Point(x, y));
		circle(raw, Point(x, y), 3, Scalar(255, 0, 0), 2);
		cout << "You need " << numChecks << " items. Currently there are " << checkedPositions.size() << endl;
		cout << "Press a key to continue" << endl;
		line(raw, Point(skalaStart, 0), Point(skalaStart, raw.rows), Scalar(0, 255, 0), 2);
		line(raw, Point(skalaEnd, 0), Point(skalaEnd, raw.rows), Scalar(0, 255, 0), 2);
		namedWindow("raw", CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED | CV_WINDOW_NORMAL);
		imshow("raw", raw);
	}
	if (event == EVENT_RBUTTONUP)
	{
		Point p1(x, y);
		int minDistIdx;
		double minDist = DBL_MAX;
		for (int i = 0; i < checkedPositions.size(); i++){
			Point p0 = checkedPositions.at(i);
			Point difference = p0 - p1;
			double distance = sqrt(difference.ddot(difference));
			if (distance < minDist){
				minDist = distance;
				minDistIdx = i;
			}
		}
		checkedPositions.erase(checkedPositions.begin() + minDistIdx);
		raw = original.clone();
		for (Point p : checkedPositions){
			circle(raw, p, 3, Scalar(0, 0, 255), 2);
		}
		cout << "You need " << numChecks << " items. Currently there are " << checkedPositions.size() << endl;
		cout << "Press a key to continue" << endl;
		line(raw, Point(skalaStart, 0), Point(skalaStart, raw.rows), Scalar(0, 255, 0), 2);
		line(raw, Point(skalaEnd, 0), Point(skalaEnd, raw.rows), Scalar(0, 255, 0), 2);
		imshow("raw", raw);
	}
	
}

bool sortPoints(Point i, Point j) { return i.y > j.y; }

static void CallBackFunc(int event, int x, int y, int, void* userdata){
	SurveyScanner* scanner = reinterpret_cast<SurveyScanner*>(userdata);
	scanner->mouseHandler(event, x, y);
}

int SurveyScanner::preProcessFrameWithReference(string path){
	std::size_t found = path.find_last_of("/\\");
	cout <<endl << "Processing frame with reference image: " << path.substr(found+1, path.length() - found-1) << endl;
	raw = imread(path);
	resize(raw, raw, rSize);
	original = raw.clone();
	cvtColor(raw, frame, CV_BGR2GRAY);
	adaptiveThreshold(~frame, frame, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);
	//frame = frame < 250;
	//namedWindow("ref", CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED | CV_WINDOW_NORMAL);
	namedWindow("binary", CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED | CV_WINDOW_NORMAL);
	

	//dilate(reference, reference, Mat::ones(Size(rSize.width / 500, rSize.width / 300), CV_8U), Point(-1, -1));
	//imshow("binary", frame);
	//imshow("ref", reference);
	//waitKey();
	frame = frame - reference;

	erode(frame, frame, Mat::ones(Size(rSize.width / 500, rSize.width / 500), CV_8U), Point(-1, -1));
	dilate(frame, frame, Mat::ones(Size(rSize.width / 10, rSize.width / 10), CV_8U), Point(-1, -1));
	erode(frame, frame, Mat::ones(Size(rSize.width / 10, rSize.width / 10), CV_8U), Point(-1, -1));

	imshow("binary", frame);
	//waitKey();
	


	return -1;
}

int SurveyScanner::process(){
	namedWindow("raw", CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED | CV_WINDOW_NORMAL);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(frame.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	checkedPositions.clear();
	for (int i = 0; i< contours.size(); i++)
	{
		// Find Center of mass of contour 
		Moments p = moments(contours.at(contours.size() - i - 1), false);
		int x = (int)(p.m10 / p.m00);
		int y = (int)(p.m01 / p.m00);
		double result = round((static_cast<double>(x)-skalaStart) * 20.0 / (skalaEnd - skalaStart));
		Point p1(x, y);
		if (x > skalaEnd + 10 || x < skalaStart - 10)
			continue;
		bool hasNeighbor = false;
		for (Point p0 : checkedPositions){
			Point difference = p0 - p1;
			double distance = sqrt(difference.ddot(difference));
			if (distance < rSize.height/50)
				hasNeighbor = true;
			double result2 = round((static_cast<double>(p0.x) - skalaStart) * 20.0 / (skalaEnd - skalaStart));
			if (result == result2 && abs(p0.y - p1.y) < (skalaEnd - skalaStart) / 10)
				hasNeighbor = true;
		}
		if (hasNeighbor)
			continue;
		cout << "Ticked number: " << result << endl;
		checkedPositions.push_back(Point(x, y));
		circle(raw, Point(x, y), 3, Scalar(0, 0, 255), 2);
	}
	line(raw, Point(skalaStart, 0), Point(skalaStart, raw.rows), Scalar(0, 255, 0), 2);
	line(raw, Point(skalaEnd, 0), Point(skalaEnd, raw.rows), Scalar(0, 255, 0), 2);
	imshow("raw", raw);
	if ((checkedPositions.size() != numChecks) || (this->mode == 0)){
		if ((checkedPositions.size() != numChecks))
			cout << "Something went wrong. Please add missing circles with left mouse button or remove wrong circles using left mouse button." << endl;
		cout << "You need " << numChecks << " items. Currently there are " << checkedPositions.size() << endl;
		cout << "Press a key to continue" << endl;
		setMouseCallback("raw", CallBackFunc, this);
		waitKey();
	}
	return -1;
}