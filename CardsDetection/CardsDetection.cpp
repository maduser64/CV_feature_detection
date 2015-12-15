#include "CardsDetection.h"

using namespace cv;
using namespace std;

int MAX_KERNEL_SIZE = 7;

void cardToVertical(Point2f* points) {
	
	float dist1 = distancePoints(points[0], points[1]);
	float dist2 = distancePoints(points[1], points[2]);

	if (dist1 > dist2) {
		Point2f p0 = points[0];

		points[0] = points[1];
		points[1] = points[2];
		points[2] = points[3];
		points[3] = p0;
	}
}

float distancePoints(Point2f p1, Point2f p2) {
	float xd = p2.x - p1.x;
	float yd = p2.y - p1.y;
	return sqrt(pow(xd,2) + pow(yd, 2));
}

Mat appendImages(Mat deckArray, Mat card, int index) {
	for (int i = 0; i < card.cols; i++) {
		for (int j = 0; j < card.rows; j++) {
			Vec3b intensity = card.at<Vec3b>(i, j);
			deckArray.at<Vec3b>(i, j + (index * 450)) = intensity;
		}
	}

	return deckArray;
}


void processDeck(string imagesDir) {
	Mat img = imread(imagesDir, CV_LOAD_IMAGE_COLOR);
	Mat findContoursMat;
	img.copyTo(findContoursMat);


	namedWindow("image", CV_WINDOW_AUTOSIZE);

	cvtColor(findContoursMat, findContoursMat, CV_RGB2GRAY);

	for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
		GaussianBlur(findContoursMat, findContoursMat, Size(i, i), 0.0, 0.0);

	threshold(findContoursMat, findContoursMat, 120, 255, CV_THRESH_BINARY);

	Canny(findContoursMat, findContoursMat, 50, 250);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// Searches for possible contours
	findContours(findContoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	// Sort the contours by area
	sort(contours.begin(), contours.end(), compareAreas);

	Mat lambda;
	Mat deckArray(450, 54*450, CV_8UC3);
	Mat procCards[54];
	Point2f outputQuad[4] = { Point2f(0, 0), Point2f(450 - 1, 0), Point2f(450 - 1, 450 - 1), Point2f(0, 450 - 1) };

	if (contours.size() >= 54) {
		Scalar color = Scalar(255, 0, 0);
		// draw the first 4 contours, this will hopefuly represent the 4 cards
		for (unsigned int i = 0; i < 54; i++) {
			//drawContours(img, contours, i, color, 4, 8, hierarchy, 0);
			RotatedRect rotatedRect = minAreaRect(contours[i]);
			Point2f rect_points[4];

			rotatedRect.points(rect_points);
			cardToVertical(rect_points);


			lambda = getPerspectiveTransform(rect_points, outputQuad);
			warpPerspective(img, procCards[i], lambda, Size(450, 450));

			appendImages(deckArray, procCards[i], i);
		}

		cvtColor(deckArray, deckArray, CV_RGB2GRAY);

		for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
			GaussianBlur(deckArray, deckArray, Size(i, i), 0.0, 0.0);

		adaptiveThreshold(deckArray, deckArray, 255, 1, 1, 11, 1);

		imshow("image", deckArray);

	}
}

void imageBasedVersion(string imagesDir) {
	Mat img = imread(imagesDir, CV_LOAD_IMAGE_COLOR);
	Mat findContoursMat;
	img.copyTo(findContoursMat);

	namedWindow("image", CV_WINDOW_AUTOSIZE);

	cvtColor(findContoursMat, findContoursMat, CV_RGB2GRAY);

	for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
		GaussianBlur(findContoursMat, findContoursMat, Size(i, i), 0.0, 0.0);

	threshold(findContoursMat, findContoursMat, 120, 255, CV_THRESH_BINARY);

	Canny(findContoursMat, findContoursMat, 50, 250);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// Searches for possible contours
	findContours(findContoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	// Sort the contours by area
	sort(contours.begin(), contours.end(), compareAreas);

	Mat lambda;
	Mat procCards[4];
	Point2f outputQuad[4] = { Point2f(0, 0), Point2f(450 - 1, 0), Point2f(450 - 1, 450 - 1), Point2f(0, 450 - 1) };

	if (contours.size() >= 4) {
		Scalar color = Scalar(255, 0, 0);
		// draw the first 4 contours, this will hopefuly represent the 4 cards
		for (unsigned int i = 0; i < 4; i++) {
			//drawContours(img, contours, i, color, 4, 8, hierarchy, 0);
			RotatedRect rotatedRect = minAreaRect(contours[i]);
			Point2f rect_points[4];

			rotatedRect.points(rect_points);
			cardToVertical(rect_points);


			lambda = getPerspectiveTransform(rect_points, outputQuad);
			warpPerspective(img, procCards[i], lambda, Size(450, 450));
		}

		imshow("image", procCards[2]);
		
	}

	// Now we need to do the comparisons


}


void videoBasedVersion() {
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cout << "Cannot open the video cam" << endl;
		return;
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
				Scalar color = Scalar(255, 255, 255);
				drawContours(gray, contours, i, color, 4, 8, hierarchy, 0, Point());

			}
		}


		imshow("Webcam", gray);

		if (waitKey(30) == 27)	{
			cout << "esc key was pressed by user" << endl;
			break;
		}
	}
}

// receives v1 and v2 point vectors and if v1 greater v2 return true
bool compareAreas(vector<Point> v1, vector<Point> v2) {

	return contourArea(v1, true) > contourArea(v2, true);

}

