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
	this->reference = imread(reference);
	
	FileNode surveysNode = fs["Surveys"];
	FileNodeIterator it = surveysNode.begin(), it_end = surveysNode.end();
	int idx = 0;
	cout << "Surveys: " << endl;
	for (; it != it_end; ++it, idx++){
		this->surveys.push_back((string)*it);
		cout << "\t" << (string)*it << endl;
	}
}

int SurveyScanner::processFrame(string path){

}