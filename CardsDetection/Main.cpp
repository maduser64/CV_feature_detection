#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

int MAX_KERNEL_SIZE = 7;

bool compareAreas(vector<Point>, vector<Point>);

int main(int argc, char** argv) {

	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cout << "Cannot open the video cam" << endl;
		return -1;
	}

	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);
	while (true) {

		Mat frame;
		bool success = cap.read(frame);
		if (!success) {
			cout << "Cannot read a frame from video stream" << endl;
		}

		Mat gray;
		cvtColor(frame, gray, CV_RGB2GRAY);
		
		for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
			GaussianBlur(gray, frame, Size(i, i), 0.0, 0.0);

		threshold(frame, frame, 120, 255, CV_THRESH_BINARY);

		Canny(frame, frame, 50, 250);

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		findContours(frame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

		sort(contours.begin(), contours.end(), compareAreas);

		RNG rng(1000);
		if (contours.size() >= 4) {
			for (int i = 0; i < 4; i++) {
				Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
				drawContours(gray, contours, i, color, 2, 8, hierarchy, 0, Point());

			}
		}


		imshow("Webcam", gray);

		if (waitKey(30) == 27)	{
			cout << "esc key was pressed by user" << endl;
			break;
		}
	}
	return 0;
}

bool compareAreas(vector<Point> v1, vector<Point> v2) {

	return contourArea(v1, true) > contourArea(v2, true);

}