#include "CardsDetection.h"
#include <math.h>

using namespace cv;
using namespace std;

int MAX_KERNEL_SIZE = 7;
#define CARD_X_SIZE 450
#define CARD_Y_SIZE 450

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


void processDeck(string imagesDir, int mode) {
	Mat img = imread(imagesDir, CV_LOAD_IMAGE_COLOR);
	Mat findContoursMat;
	img.copyTo(findContoursMat);

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

	// Will hold the prespective transform mapping
	Mat lambda;
	// Will hold the array of cards with the entire deck
	Mat deckArray(CARD_Y_SIZE, 54 * CARD_X_SIZE, CV_8UC3);

	Mat outputCard;
	Point2f outputQuad[4] = { Point2f(0, 0), Point2f(450 - 1, 0), Point2f(450 - 1, 450 - 1), Point2f(0, 450 - 1) };

	if (contours.size() >= 54) {
		Scalar color = Scalar(255, 0, 0);
		// foreach of 54 cards represented on image
		for (unsigned int i = 0; i < 54; i++) {
			//drawContours(img, contours, i, color, 4, 8, hierarchy, 0);
			RotatedRect rotatedRect = minAreaRect(contours[i]);
			Point2f rect_points[4];
			rotatedRect.points(rect_points);
			cardToVertical(rect_points);

			lambda = getPerspectiveTransform(rect_points, outputQuad);
			warpPerspective(img, outputCard, lambda, Size(450, 450));

			appendImages(deckArray, outputCard, i);
		}

		if (mode == 0) {
			cvtColor(deckArray, deckArray, CV_RGB2GRAY);
			for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
				GaussianBlur(deckArray, deckArray, Size(i, i), 0.0, 0.0);

			adaptiveThreshold(deckArray, deckArray, 255, 1, 1, 11, 1);
			//threshold(deckArray, deckArray, 130, 255, CV_THRESH_BINARY_INV);

			imwrite("deck/deck_array_gray.jpg", deckArray);
		} else
			imwrite("deck/deck_array_color.jpg", deckArray);
	}
}

void rotateCard(Mat& src, double angle, Mat& dst) {
	Point2f pt(src.cols / 2.0F, src.rows / 2.0F);
	// Returns the rotation matrix that represents the specified rotation
	Mat r = getRotationMatrix2D(pt, angle, 1.0);

	// Applies the previous obtained matrix and returns the rotated Mat
	warpAffine(src, dst, r, src.size());
}

void rotateText(Mat& src, double angle, Point2f originPoint, Mat& dst) {

	// Returns the rotation matrix that represents the specified rotation
	Mat r = getRotationMatrix2D(originPoint, angle, 1.0);

	// Applies the previous obtained matrix and returns the rotated Mat
	warpAffine(src, dst, r, src.size());
}

bool auxMinYPointSort(Point2f pt1, Point2f pt2) { return (pt1.y < pt2.y); }

bool auxMinXPointSort(Point2f pt1, Point2f pt2) { return (pt1.x < pt2.x); }

double computeCardAngle(PlayedCard* card) {

	double pi = 3.141592653589793238462643383279502884;

	/*Min y*/
	Point2f p1MinY, p2MinY;
	p1MinY = Point2f();
	p2MinY = Point2f();

	/*Min x*/
	Point2f p1MinX, p2MinX;
	p1MinX = Point2f();
	p2MinX = Point2f();

	vector<Point2f> cardYMinPoints = card->getCornerPoints();
	vector<Point2f> cardXMinPoints = card->getCornerPoints();

	sort(cardYMinPoints.begin(), cardYMinPoints.end(), auxMinYPointSort);
	sort(cardXMinPoints.begin(), cardXMinPoints.end(), auxMinXPointSort);

	p1MinY = Point((int) cardYMinPoints[0].x, (int) cardYMinPoints[0].y);
	p2MinY = Point((int) cardYMinPoints[1].x, (int) cardYMinPoints[1].y);

	p1MinX = Point((int) cardXMinPoints[0].x, (int) cardXMinPoints[0].y);
	p2MinX = Point((int) cardXMinPoints[1].x, (int) cardXMinPoints[1].y);


	float distY = distancePoints(p1MinY, p2MinY);
	float distX = distancePoints(p1MinX, p2MinX);

	float angle = 0;
	if (distY < distX)
		angle = atan((p2MinY.y - p1MinY.y) / (p2MinY.x - p1MinY.x));
	else
		angle = atan((p2MinX.y - p1MinX.y) / (p2MinX.x - p1MinX.x));
	
	
	return -angle * (180/pi);

}

PlayedCard* getWinner(vector<PlayedCard*> playedCards) {
	PlayedCard* winnerCard = new PlayedCard();
	for (unsigned int i = 0; i < 4; i++) {
		if (i == 0)
			winnerCard = playedCards[i];
		else {
			if (playedCards[i]->getLeastDifferentCard()->getScore() > winnerCard->getLeastDifferentCard()->getScore())
				winnerCard = playedCards[i];
		}
	}

	return winnerCard;
}

Point getCenterPoint(PlayedCard* winnerCard) {

	int centerX = (int) (winnerCard->getCornerPoints()[0].x + ((winnerCard->getCornerPoints()[1].x - winnerCard->getCornerPoints()[0].x) / 2));
	int centerY = (int) (winnerCard->getCornerPoints()[1].y + ((winnerCard->getCornerPoints()[2].y - winnerCard->getCornerPoints()[1].y) / 2));

	return Point((int) winnerCard->getCornerPoints()[1].x, centerY);
}

void imageBasedVersion(string imagesDir, int mode) {

	Mat img = imread(imagesDir, CV_LOAD_IMAGE_COLOR);
	Mat findContoursMat;
	img.copyTo(findContoursMat);

	if (mode == 0) {
		cvtColor(findContoursMat, findContoursMat, CV_RGB2GRAY);
		
		for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
			GaussianBlur(findContoursMat, findContoursMat, Size(i, i), 0.0, 0.0);

		threshold(findContoursMat, findContoursMat, 120, 255, CV_THRESH_BINARY);
	}

	Canny(findContoursMat, findContoursMat, 50, 250);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// Searches for possible contours
	findContours(findContoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	// Sort the contours by area
	sort(contours.begin(), contours.end(), compareAreas);

	Mat lambda;
	
	// Holds the 4 cards on present on table
	Mat procCards[4];

	// Holds the corners coordinats of each card
	vector<vector<Point2f>> cornerPoints;

	Point2f outputQuad[4] = { Point2f(0, 0), Point2f(CARD_X_SIZE - 1, 0), Point2f(CARD_X_SIZE - 1, CARD_Y_SIZE - 1), Point2f(0, CARD_Y_SIZE - 1) };

	if (contours.size() >= 4) {
		Scalar color = Scalar(255, 0, 0);
		// draw the first 4 contours, this will hopefuly represent the 4 cards
		for (unsigned int i = 0; i < 4; i++) {
			drawContours(img, contours, i, color, 4, 8, hierarchy, 0);
			RotatedRect rotatedRect = minAreaRect(contours[i]);
			Point2f rect_points[4];

			// gets the corner points of each card
			rotatedRect.points(rect_points);
			// inserts it on a vector for later use, on card class construction


			vector<Point2f> tempPoints;
			for (unsigned int k = 0; k < 4; k++)
				tempPoints.push_back(rect_points[k]);

			cornerPoints.push_back(tempPoints);
			// Rotates card to make it vertical
			cardToVertical(rect_points);


			lambda = getPerspectiveTransform(rect_points, outputQuad);
			warpPerspective(img, procCards[i], lambda, Size(450, 450));

			if (mode == 0) {
				cvtColor(procCards[i], procCards[i], CV_RGB2GRAY);

				for (int j = 1; j < MAX_KERNEL_SIZE; j = j + 2)
					GaussianBlur(procCards[i], procCards[i], Size(j, j), 0.0, 0.0);

				adaptiveThreshold(procCards[i], procCards[i], 255, 1, 1, 11, 1);
				//threshold(procCards[i], procCards[i], 130, 255, CV_THRESH_BINARY_INV);
			}
		}		
	}

	vector<Card*> deck; deck.clear(); deck = getDeck(mode);
	vector<PlayedCard*> playedCards; playedCards.clear();

	// Save the original cards and the 180 degrees rotations
	for (unsigned int i = 0; i < 4; i++) {
		Mat rotatedCard;
		rotateCard(procCards[i], 180.0f, rotatedCard);
		// push back a new played card and 
		playedCards.push_back(new PlayedCard(procCards[i], rotatedCard, contours[i], cornerPoints[i], deck, mode));
	}


	// Show matched cards info
	for (unsigned int i = 0; i < 4; i++) {
		string card = playedCards[i]->getLeastDifferentCard()->getCard();
		string suit = playedCards[i]->getLeastDifferentCard()->getSuit();
		int score = playedCards[i]->getLeastDifferentCard()->getScore();
		cout << "Card: " << card << " | suit: " << suit << " | score: " << score << endl;

		for (int j = 0; j < 4; j++) 
			circle(img, playedCards[i]->getCornerPoints()[j], 10, Scalar(255, 0, 0), 10);
		
	}

	PlayedCard* winnerCard = getWinner(playedCards);

	cout << "--------------------------------" << endl;
	cout << winnerCard->getCornerPoints()[0] << endl;
	cout << winnerCard->getCornerPoints()[1] << endl;
	cout << winnerCard->getCornerPoints()[2] << endl;
	cout << winnerCard->getCornerPoints()[3] << endl;
	cout << "--------------------------------" << endl;

	Mat textImage = Mat::zeros(img.rows, img.cols, img.type());
	putText(textImage, "Winner!", winnerCard->getCentralPoint(), FONT_HERSHEY_SCRIPT_COMPLEX, 3.0, Scalar(0, 0, 255));

	rotateText(textImage, computeCardAngle(winnerCard), winnerCard->getCentralPoint(), textImage);
	img = img + textImage;

	namedWindow("Image", WINDOW_NORMAL);
	imshow("Image", img);
}


void videoBasedVersion(int mode) {

	Mat image;
	int state = -1;
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
		imshow("Webcam", frame);

		if (waitKey(10) == 32) {
			cout << "Image saved" << endl;
			image = frame;
			state = 1;
			break;
		}
		if (waitKey(100) == 27)	{
			cout << "esc key pressed" << endl;
			state = 0;
			break;
		}
	}

	if (state == 1)
		processVideo(image, mode);
}

void processVideo(Mat frame, int mode) {
	Mat img = frame;
	Mat findContoursMat;
	img.copyTo(findContoursMat);

	if (mode == 0) {
		cvtColor(findContoursMat, findContoursMat, CV_RGB2GRAY);

		for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
			GaussianBlur(findContoursMat, findContoursMat, Size(i, i), 0.0, 0.0);

		threshold(findContoursMat, findContoursMat, 120, 255, CV_THRESH_BINARY);
	}

	Canny(findContoursMat, findContoursMat, 50, 250);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// Searches for possible contours
	findContours(findContoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	// Sort the contours by area
	sort(contours.begin(), contours.end(), compareAreas);

	Mat lambda;

	// Holds the 4 cards on present on table
	Mat procCards[4];

	// Holds the corners coordinats of each card
	vector<vector<Point2f>> cornerPoints;

	Point2f outputQuad[4] = { Point2f(0, 0), Point2f(CARD_X_SIZE - 1, 0), Point2f(CARD_X_SIZE - 1, CARD_Y_SIZE - 1), Point2f(0, CARD_Y_SIZE - 1) };

	if (contours.size() >= 4) {
		Scalar color = Scalar(255, 0, 0);
		// draw the first 4 contours, this will hopefuly represent the 4 cards
		for (unsigned int i = 0; i < 4; i++) {
			drawContours(img, contours, i, color, 4, 8, hierarchy, 0);
			RotatedRect rotatedRect = minAreaRect(contours[i]);
			Point2f rect_points[4];

			// gets the corner points of each card
			rotatedRect.points(rect_points);
			// inserts it on a vector for later use, on card class construction

			vector<Point2f> tempPoints;
			for (unsigned int k = 0; k < 4; k++)
				tempPoints.push_back(rect_points[k]);

			cornerPoints.push_back(tempPoints);
			// Rotates card to make it vertical
			cardToVertical(rect_points);

			lambda = getPerspectiveTransform(rect_points, outputQuad);
			warpPerspective(img, procCards[i], lambda, Size(450, 450));

			if (mode == 0) {
				cvtColor(procCards[i], procCards[i], CV_RGB2GRAY);

				for (int j = 1; j < MAX_KERNEL_SIZE; j = j + 2)
					GaussianBlur(procCards[i], procCards[i], Size(j, j), 0.0, 0.0);

				adaptiveThreshold(procCards[i], procCards[i], 255, 1, 1, 11, 1);
				//threshold(procCards[i], procCards[i], 130, 255, CV_THRESH_BINARY_INV);
			}
		}
	}

	vector<Card*> deck; deck.clear(); deck = getDeck(mode);
	vector<PlayedCard*> playedCards; playedCards.clear();

	// Save the original cards and the 180 degrees rotations
	for (unsigned int i = 0; i < 4; i++) {
		Mat rotatedCard;
		rotateCard(procCards[i], 180.0f, rotatedCard);
		// push back a new played card and 
		playedCards.push_back(new PlayedCard(procCards[i], rotatedCard, contours[i], cornerPoints[i], deck, mode));
	}

	// Show matched cards info
	for (unsigned int i = 0; i < 4; i++) {
		string card = playedCards[i]->getLeastDifferentCard()->getCard();
		string suit = playedCards[i]->getLeastDifferentCard()->getSuit();
		int score = playedCards[i]->getLeastDifferentCard()->getScore();
		cout << "Card: " << card << " | suit: " << suit << " | score: " << score << endl;

		for (int j = 0; j < 4; j++)
			circle(img, playedCards[i]->getCornerPoints()[j], 10, Scalar(255, 0, 0), 10);
	}

	PlayedCard* winnerCard = getWinner(playedCards);

	cout << "--------------------------------" << endl;
	cout << winnerCard->getCornerPoints()[0] << endl;
	cout << winnerCard->getCornerPoints()[1] << endl;
	cout << winnerCard->getCornerPoints()[2] << endl;
	cout << winnerCard->getCornerPoints()[3] << endl;
	cout << "--------------------------------" << endl;

	Mat textImage = Mat::zeros(img.rows, img.cols, img.type());
	putText(textImage, "Winner!", winnerCard->getCentralPoint(), FONT_HERSHEY_SCRIPT_COMPLEX, 3.0, Scalar(0, 0, 255));

	rotateText(textImage, computeCardAngle(winnerCard), winnerCard->getCentralPoint(), textImage);
	img = img + textImage;

	namedWindow("Image", WINDOW_NORMAL);
	imshow("Image", img);

	if (continueWebcam())
		videoBasedVersion(mode);
	else
		return;
}

// receives v1 and v2 point vectors and if v1 greater v2 return true
bool compareAreas(vector<Point> v1, vector<Point> v2) {
	return contourArea(v1, true) > contourArea(v2, true);
}
