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
	skalaStart = (double)fs["SkalaStartPx"];
	cout << "Skala start Pixel: " << endl << "\t" << skalaStart << endl;
	skalaEnd = (double)fs["SkalaEndPx"];
	cout << "Skala end Pixel: " << endl << "\t" << skalaEnd << endl;
	numChecks = (int)fs["NumberOfItems"];
	cout << "Number Of Items: " << endl << "\t" << numChecks << endl;
	string mode = fs["Mode"];
	if (mode == "semi")
		this->mode = 0;
	if (mode == "automatic")
		this->mode = 1;
	Mat raw = imread(reference);
	cvtColor(raw, this->reference, CV_BGR2GRAY);
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
		processFrameWithReference(survey);
	}
}


//This is not used/correct yet.  I tried to implement a check detection by removing the skala (= horizontal and vertical lines) from the image.
int SurveyScanner::processFrame(string path){
	std::size_t found = path.find_last_of("/\\");
	cout << endl << "Processing frame " << path.substr(found, path.length() - found) << endl;
	Mat raw = imread(path);
	Mat frame;
	cvtColor(raw, frame, CV_BGR2GRAY);
	adaptiveThreshold(~frame, frame, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

	Mat horizontal = frame.clone();
	Mat vertical = frame.clone();

	// Specify size on horizontal axis
	int horizontalsize = horizontal.cols / 30;
	// Create structure element for extracting horizontal lines through morphology operations
	Mat horizontalStructure = getStructuringElement(MORPH_RECT, Size(horizontalsize, 1));
	// Apply morphology operations
	erode(horizontal, horizontal, horizontalStructure, Point(-1, -1));
	dilate(horizontal, horizontal, horizontalStructure, Point(-1, -1));
	// Show extracted horizontal lines
	imshow("horizontal", horizontal);

	// Specify size on vertical axis
	int verticalsize = vertical.rows / 90;
	// Create structure element for extracting vertical lines through morphology operations
	Mat verticalStructure = getStructuringElement(MORPH_RECT, Size(1, verticalsize));
	// Apply morphology operations
	erode(vertical, vertical, verticalStructure, Point(-1, -1));
	dilate(vertical, vertical, verticalStructure, Point(-1, -1));
	// Show extracted vertical lines
	imshow("vertical", vertical);

	

	frame = frame - vertical;
	frame = frame - horizontal;
	
	erode(frame, frame, Mat::ones(Size(1,1), CV_8U), Point(-1, -1));
	//dilate(frame, frame, Mat::ones(Size(7,7), CV_8U), Point(-1, -1), 1);
	//frame = vertical + horizontal;
	
	imshow("binary", frame);
	waitKey();


	return -1;
}

void SurveyScanner::mouseHandler(int event, int x, int y){
	if (event == EVENT_LBUTTONUP)
	{
		double result = round((static_cast<double>(x)-skalaStart) * 20.0 / (skalaEnd - skalaStart));
		cout << "Ticked number: " << result << endl;
		checkedPositions.push_back(Point(x, y));
		circle(raw, Point(x, y), 3, Scalar(255, 0, 0), 2);
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
		
	}
	imshow("raw", raw);
}

bool sortPoints(Point i, Point j) { return i.y > j.y; }

static void CallBackFunc(int event, int x, int y, int, void* userdata){
	SurveyScanner* scanner = reinterpret_cast<SurveyScanner*>(userdata);
	scanner->mouseHandler(event, x, y);
}

int SurveyScanner::processFrameWithReference(string path){
	std::size_t found = path.find_last_of("/\\");
	cout <<endl << "Processing frame with reference image: " << path.substr(found+1, path.length() - found-1) << endl;
	raw = imread(path);
	Mat frame;
	original = raw.clone();
	cvtColor(raw, frame, CV_BGR2GRAY);
	adaptiveThreshold(~frame, frame, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

	frame = frame - reference;

	dilate(frame, frame, Mat::ones(Size(10,10), CV_8U), Point(-1, -1));
	erode(frame, frame, Mat::ones(Size(10, 10), CV_8U), Point(-1, -1));

	//imshow("binary", frame);
	//waitKey();
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(frame.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	checkedPositions.clear();
	for (int i = 0; i< contours.size(); i++)
	{
		// Find Center of mass of contour 
		Moments p = moments(contours.at(contours.size()-i-1), false);
		int x = (int)(p.m10 / p.m00);
		int y = (int)(p.m01 / p.m00);
		double result = round((static_cast<double>(x)-skalaStart) * 20.0 / (skalaEnd - skalaStart));
		Point p1(x, y);
		bool hasNeighbor = false;
		for (Point p0 : checkedPositions){
			//Point difference = p0 - p1;
			//double distance = sqrt(difference.ddot(difference));
			//if (distance < 8)
			//	hasNeighbor = true;
			double result2 = round((static_cast<double>(p0.x)-skalaStart) * 20.0 / (skalaEnd - skalaStart));
			if (result == result2 && abs(p0.y - p1.y) < 20)
				hasNeighbor = true;
		}
		if (hasNeighbor)
			continue;
		cout << "Ticked number: " << result << endl;
		checkedPositions.push_back(Point(x, y));
		circle(raw, Point(x, y), 3, Scalar(0,0,255), 2);
	}
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