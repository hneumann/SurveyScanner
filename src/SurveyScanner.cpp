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

int SurveyScanner::processFrameWithReference(string path){
	std::size_t found = path.find_last_of("/\\");
	cout <<endl << "Processing frame with reference image " << path.substr(found+1, path.length() - found-1) << endl;
	Mat raw = imread(path);
	Mat frame;
	cvtColor(raw, frame, CV_BGR2GRAY);
	adaptiveThreshold(~frame, frame, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

	frame = frame - reference;

	dilate(frame, frame, Mat::ones(Size(10,10), CV_8U), Point(-1, -1));
	erode(frame, frame, Mat::ones(Size(10, 10), CV_8U), Point(-1, -1));

	imshow("binary", frame);
	//waitKey();
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(frame.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	for (int i = 0; i< contours.size(); i++)
	{
		// Find Center of mass of contour 
		Moments p = moments(contours.at(i), false);
		int x = (int)(p.m10 / p.m00);
		int y = (int)(p.m01 / p.m00);
		double result = round((static_cast<double>(x) - 337.0) * 20.0 / 183.0);
		cout << "Ticked number: " << result << endl;
		circle(raw, Point(x, y), 3, Scalar(255,0,0), 2);
	}


	imshow("raw", raw);
	waitKey();


	return -1;
}