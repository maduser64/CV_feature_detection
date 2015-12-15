#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Cli.h"
#include "Opencv.h"
#include "CardsDetection.h"
#include "Crop.h"
#include "DeckReader.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {

	string contentDir = "cards\\";
	string test_dir = "test_samples\\";

	Mat img = imread("cards/fulldeck.png", CV_LOAD_IMAGE_COLOR);

	/*
	// Crop the original cards image
	for (int i = 0; i < 4; i++)
		for (int j = 3; j < 14; j++){
			Mat cropped = crop(192 * i, 288 * j, img);
			imshow("Display window" + to_string(i) + to_string(j), cropped);
		}

	waitKey(0);
	*/

	int choice = -1; // 1-> image / 2 -> video
	choice = initCli();

	string imgPath = "";
	switch (choice) {
		case 1:
			imgPath = getImgPath(test_dir);
			/*processDeck(imgPath);*/
			imageBasedVersion(imgPath);
			break;
		case 2:
			videoBasedVersion();
			break;
		default:
			break;
	}

	waitKey(0);
	return 0;
}