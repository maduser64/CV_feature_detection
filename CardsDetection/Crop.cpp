#include "Crop.h"

using namespace std;
using namespace cv;

Mat crop(int start_x, int start_y, Mat original) {

	// rectagle represents the card position and size
	cv::Rect myROI(start_x, start_y, 192, 288);

	cv::Mat cropped = original(myROI);

	return cropped;

}