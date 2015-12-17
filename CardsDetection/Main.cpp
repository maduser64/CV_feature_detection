#include <iostream>
#include "Cli.h"
#include "Opencv.h"
#include "CardsDetection.h"
#include "Crop.h"
#include "DeckReader.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
	string test_dir = "test_samples\\";
	Mat img = imread("cards/fulldeck.png", CV_LOAD_IMAGE_COLOR); 

	int mode = -1;
	mode = chooseExecutionMode();

	if (!deckPreProcessed(mode)) {
		cout << "Pre-processing deck, please wait" << endl;
		string fullDeckPath = "deck/deck.jpg";
		processDeck(fullDeckPath, mode);
		cout << "Success!" << endl;
	}

	int choice = -1; // 1-> image / 2 -> video
	choice = initCli();
	string imgPath = "";
	switch (choice) {
		case 1:
			imgPath = getImgPath(test_dir);
			imageBasedVersion(imgPath, mode);
			break;
		case 2:
			videoBasedVersion(mode);
			break;
		default:
			break;
	}

	waitKey(0);
	return 0;
}