#include "CardsDetection.h"

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


void processDeck(string imagesDir) {
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

		cvtColor(deckArray, deckArray, CV_RGB2GRAY);

		for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
			GaussianBlur(deckArray, deckArray, Size(i, i), 0.0, 0.0);

		adaptiveThreshold(deckArray, deckArray, 255, 1, 1, 11, 1);

		//threshold(deckArray, deckArray, 130, 255, CV_THRESH_BINARY_INV);

		imwrite("deck/deck_array.jpg", deckArray);

	}
}

void rotateCard(Mat& src, double angle, Mat& dst) {
	Point2f pt(src.cols / 2.0F, src.rows / 2.0F);
	// Returns the rotation matrix that represents the specified rotation
	Mat r = getRotationMatrix2D(pt, angle, 1.0);

	// Applies the previous obtained matrix and returns the rotated Mat
	warpAffine(src, dst, r, src.size());
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
	int centerX = (int) winnerCard->getCornerPoints()[0].x + (int) ((winnerCard->getCornerPoints()[1].x - winnerCard->getCornerPoints()[0].x) / 2);
	int centerY = (int) winnerCard->getCornerPoints()[1].y + (int) ((winnerCard->getCornerPoints()[2].y - winnerCard->getCornerPoints()[1].y) / 2);

	return Point((int) winnerCard->getCornerPoints()[1].x, centerY);
}

void imageBasedVersion(string imagesDir) {
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

			cvtColor(procCards[i], procCards[i], CV_RGB2GRAY);

			for (int j = 1; j < MAX_KERNEL_SIZE; j = j + 2)
				GaussianBlur(procCards[i], procCards[i], Size(j, j), 0.0, 0.0);

			adaptiveThreshold(procCards[i], procCards[i], 255, 1, 1, 11, 1);
			//threshold(procCards[i], procCards[i], 130, 255, CV_THRESH_BINARY_INV);
		}		
	}

	vector<Card*> deck; deck.clear(); deck = getDeck();
	vector<PlayedCard*> playedCards; playedCards.clear();


	//Point2f points[] = { Point2f(1.0f, 2.0f), Point2f(1.0f, 2.0f), Point2f(1.0f, 2.0f), Point2f(1.0f, 2.0f) };
	// Save the original cards and the 180 degrees rotations

	for (unsigned int i = 0; i < 4; i++) {
		Mat rotatedCard;
		rotateCard(procCards[i], 180.0f, rotatedCard);
		// push back a new played card and 
		playedCards.push_back(new PlayedCard(procCards[i], rotatedCard, cornerPoints[i], deck));
	}


	// Show matched cards info
	for (unsigned int i = 0; i < 4; i++) {
		string card = playedCards[i]->getLeastDifferentCard()->getCard();
		string suit = playedCards[i]->getLeastDifferentCard()->getSuit();
		int score = playedCards[i]->getLeastDifferentCard()->getScore();

		cout << "Card: " << card << " | suit: " << suit << " | score: " << score << endl;
	}

	
	PlayedCard* winnerCard = getWinner(playedCards);

	cout << "--------------------------------" << endl;
	cout << winnerCard->getCornerPoints()[0] << endl;
	cout << winnerCard->getCornerPoints()[1] << endl;
	cout << winnerCard->getCornerPoints()[2] << endl;
	cout << winnerCard->getCornerPoints()[3] << endl;
	cout << "--------------------------------" << endl;


	putText(img, "Winner!", getCenterPoint(winnerCard), FONT_HERSHEY_SCRIPT_COMPLEX, 3.0, Scalar(0, 0, 255));



	namedWindow("Conas", WINDOW_NORMAL);
	imshow("Conas", img);
	/*
	namedWindow("Contours", WINDOW_NORMAL);
	imshow("Contours", img);
	
	namedWindow("Original", WINDOW_AUTOSIZE);
	imshow("Original", playedCards[0]->getOriginalImg());

	Card* card = playedCards[0]->getLeastDifferentCard();
	namedWindow("CardLeast", WINDOW_AUTOSIZE);
	imshow("CardLeast", card->getCardImg());

	
	
	namedWindow("Original1", WINDOW_AUTOSIZE);
	imshow("Original1", playedCards[1]->getOriginalImg());

	Card* card1 = playedCards[1]->getLeastDifferentCard();
	namedWindow("CardLeast1", WINDOW_AUTOSIZE);
	imshow("CardLeast1", card1->getCardImg());

	namedWindow("Original2", WINDOW_AUTOSIZE);
	imshow("Original2", playedCards[2]->getOriginalImg());

	Card* card2 = playedCards[2]->getLeastDifferentCard();
	namedWindow("CardLeast2", WINDOW_AUTOSIZE);
	imshow("CardLeast2", card2->getCardImg());

	namedWindow("Original3", WINDOW_AUTOSIZE);
	imshow("Original3", playedCards[3]->getOriginalImg());

	Card* card3 = playedCards[3]->getLeastDifferentCard();
	namedWindow("CardLeast3", WINDOW_AUTOSIZE);
	imshow("CardLeast3", card3->getCardImg());
*/
	
	


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
