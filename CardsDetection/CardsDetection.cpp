#include "CardsDetection.h"
#include <math.h>

using namespace cv;
using namespace std;

int MAX_KERNEL_SIZE = 7;
#define CARD_X_SIZE 450
#define CARD_Y_SIZE 450

bool compareAreas(vector<Point> v1, vector<Point> v2) {
	return contourArea(v1, true) > contourArea(v2, true);
}

bool compareAreasF(vector<Point2f> v1, vector<Point2f> v2) {
	return contourArea(v1, true) > contourArea(v2, true);
}

bool compareIndex(PointOrder p1, PointOrder p2) {
	return (p1.order < p2.order);
}

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
	int progress = 0;	

	Mat img = imread(imagesDir, CV_LOAD_IMAGE_COLOR); progress += 2; showLoadingBar(progress);
	Mat findContoursMat;
	img.copyTo(findContoursMat); progress += 2; showLoadingBar(progress);

	cvtColor(findContoursMat, findContoursMat, CV_RGB2GRAY); progress += 2; showLoadingBar(progress);

	for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
		GaussianBlur(findContoursMat, findContoursMat, Size(i, i), 0.0, 0.0); for (int i = 0; i < 5; i++) { progress += 2; showLoadingBar(progress); Sleep(10); }

	threshold(findContoursMat, findContoursMat, 120, 255, CV_THRESH_BINARY); for (int i = 0; i < 5; i++) { progress += 2; showLoadingBar(progress); Sleep(10); }

	Canny(findContoursMat, findContoursMat, 50, 250); for (int i = 0; i < 5; i++) { progress += 2; showLoadingBar(progress); Sleep(10); }

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// Searches for possible contours
	findContours(findContoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE); for (int i = 0; i < 5; i++) { progress += 2; showLoadingBar(progress); Sleep(10); }

	// Sort the contours by area
	sort(contours.begin(), contours.end(), compareAreas); for (int i = 0; i < 2; i++) { progress += 2; showLoadingBar(progress); Sleep(10); }

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
			RotatedRect rotatedRect = minAreaRect(contours[i]);
			Point2f rect_points[4];
			rotatedRect.points(rect_points);
			cardToVertical(rect_points);

			lambda = getPerspectiveTransform(rect_points, outputQuad);
			warpPerspective(img, outputCard, lambda, Size(450, 450));

			appendImages(deckArray, outputCard, i);

			if (i < 50) {
				progress += 1;
				showLoadingBar(progress);
			}
		}
		cout << "\n";

		if (mode == 0) {
			cvtColor(deckArray, deckArray, CV_RGB2GRAY);
			for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
				GaussianBlur(deckArray, deckArray, Size(i, i), 0.0, 0.0);

			adaptiveThreshold(deckArray, deckArray, 255, 1, 1, 11, 1);
			//threshold(deckArray, deckArray, 130, 255, CV_THRESH_BINARY_INV);

			imwrite("deck/deck_array_gray.jpg", deckArray);
		}
		else
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

void getWinner(vector<PlayedCard*> playedCards) {
	PlayedCard* winnerCard = new PlayedCard();
	int winnerScore = 0;

	// set the highest score
	for (unsigned int i = 0; i < 4; i++) {
		if (i == 0) {
			winnerCard = playedCards[i];
			winnerScore = playedCards[i]->getLeastDifferentCard()->getScore();
		}
		else {
			if (playedCards[i]->getLeastDifferentCard()->getScore() > winnerCard->getLeastDifferentCard()->getScore()) {
				winnerCard = playedCards[i];
				winnerScore = winnerCard->getLeastDifferentCard()->getScore();
			}
				
		}
	}

	PlayedCard* card = new PlayedCard();
	int winnerCounter = 0;
	
	// Mark card as winner, loser
	for (unsigned int i = 0; i < 4; i++) {
		card = playedCards[i];
		if (card->getLeastDifferentCard()->getScore() >= winnerScore) {
			card->setWinner('w');
			winnerCounter++;
		} else card->setWinner('l');
	}
	
	// There's a draw
	if (winnerCounter > 1) {
		for (unsigned int i = 0; i < 4; i++) {
			if (playedCards[i]->getWinner() == 'w')
				playedCards[i]->setWinner('d');
		}
	}
}

void imageBasedVersion(string imagesDir, int mode, int choice) {

	Mat img = imread(imagesDir, CV_LOAD_IMAGE_COLOR);
	img = resizeImage(img, Size(1000, 700));
	Mat findContoursMat;
	img.copyTo(findContoursMat);

	cvtColor(findContoursMat, findContoursMat, CV_RGB2GRAY);

	/**********************Denoise Image**********************/
	for (int i = 1; i < MAX_KERNEL_SIZE; i = i + 2)
		GaussianBlur(findContoursMat, findContoursMat, Size(i, i), 0.0, 0.0);

	threshold(findContoursMat, findContoursMat, 120, 255, CV_THRESH_BINARY);
	/*********************************************************/

	Canny(findContoursMat, findContoursMat, 50, 250);

	// Each element of this vector represents a contour of each closed shape detected
	vector<vector<Point> > contours;
	vector<vector<Point> > polyContours;
	vector<Vec4i> hierarchy;

	// Searches for possible contours and stores them hierarchically and eliminates redundant points
	findContours(findContoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	

	// Sort the contours by area
	sort(contours.begin(), contours.end(), compareAreas);

	Mat lambda;
	
	// Holds the 4 cards on present on table
	Mat procCards[4];
	Point centerPoints[4];
	

	// Holds the corners coordinats of each card
	vector<vector<Point2f> > cornersNew;
	vector<vector<Point2f> > cornersVertical;

	Point2f outputQuad[4] = { Point2f(0, 0), Point2f(CARD_X_SIZE - 1, 0), Point2f(CARD_X_SIZE - 1, CARD_Y_SIZE - 1), Point2f(0, CARD_Y_SIZE - 1) };

	if (contours.size() >= 4) {		
		// draw the first 4 contours, this will hopefuly represent the 4 cards
		for (unsigned int i = 0; i < 4; i++) {
			// Get less contours
			vector<Point> contour;
			approxPolyDP(contours[i], contour, 1, true);
			polyContours.push_back(contour);

			// find card center points
			centerPoints[i] = computeCentralPoint(polyContours[i]);


			// ordered corners
			cornersNew.push_back(getCardCorners(centerPoints[i], polyContours[i], choice));
			

			// convert vector to array
			Point2f* newCornerArray = &cornersNew[i][0];

			// card is vertical
			cardToVertical(newCornerArray);

			// convert back to vector
			vector<Point2f> verticalAux;
			for (int k = 0; k < 4; k++) {
				verticalAux.push_back(newCornerArray[k]);
			}

			// card in now vertical
			cornersVertical.push_back(verticalAux);

			
			// Calculates a perspective transform from four pairs of the corresponding points
			lambda = getPerspectiveTransform(newCornerArray, outputQuad);
			// Applies a prespective transformation to an image
			warpPerspective(img, procCards[i], lambda, Size(450, 450));

			// Reduce noise and do an equal treatment 
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
		// push back a new played card and compute the match
		playedCards.push_back(new PlayedCard(procCards[i], rotatedCard, contours[i], cornersVertical[i], deck, mode, i));
	}

	getWinner(playedCards);

	// Show matched cards info
	cout << "\n\n Results: " << endl;
	for (unsigned int i = 0; i < 4; i++) {
		string card = playedCards[i]->getLeastDifferentCard()->getCard();
		string suit = playedCards[i]->getLeastDifferentCard()->getSuit();
		int score = playedCards[i]->getLeastDifferentCard()->getScore();
		cout << "Card: " << card << " | suit: " << suit << " | score: " << score << endl;

		// write info about cards
		playedCards[i]->drawCardText(img);
		
	}

	for (unsigned int i = 0; i < 4; i++) {
		if (playedCards[i]->getWinner() == 'w') {
			cout << "\n Winner: " << endl;
			string card = playedCards[i]->getLeastDifferentCard()->getCard();
			string suit = playedCards[i]->getLeastDifferentCard()->getSuit();
			int score = playedCards[i]->getLeastDifferentCard()->getScore();
			cout << "Card: " << card << " | suit: " << suit << " | score: " << score << endl;
		}
	}

	namedWindow("Results", WINDOW_AUTOSIZE);
	Scalar color = Scalar(255, 0, 0);
	for (int a = 0; a < 4; a++) {
		drawContours(img, contours, a, color, 4, 8, hierarchy, 0);
		
		for (int b = 0; b < 4; b++)
			circle(img, cornersVertical[a][b], 1, Scalar(0, 0, 255), 5);
	}
	
	imshow("Results", img);
}

void videoBasedVersion(int mode, int choice) {

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

		if (waitKey(30) == 32) {
			cout << "Image saved" << endl;
			image = frame;
			state = 1;
			break;
		}
	}

	destroyWindow("Webcam");
	if (state == 1)
		processVideo(image, mode, choice);
}

void processVideo(Mat frame, int mode, int choice) {
	
	time_t timer;
	struct tm y2k = { 0 };
	double seconds;
	y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
	time(&timer); 
	seconds = difftime(timer, mktime(&y2k));

	stringstream ss;
	ss << "test_samples/frame" << seconds << ".jpg";
	string fileName = ss.str();
	imwrite(fileName, frame);	

	imageBasedVersion(fileName, mode, choice);

	waitKey(0);

	if (continueWebcam()) {
		destroyAllWindows();
		videoBasedVersion(mode, choice);
	}
	else
		destroyAllWindows();
}

Mat resizeImage(Mat src, Size size) {

	// The image is already smaller
	if (src.size().width <= size.width && src.size().height <= size.height)
		return src;

	float wRatio = (float) src.size().width / size.width;
	float hRatio = (float) src.size().height / size.height;
	
	int newWidth;
	int newHeight;
	Mat resized;

	if (wRatio >= hRatio) {
		newWidth = size.width;
		newHeight = (int) (src.size().height / wRatio);
	}
	else {
		newHeight = size.height;
		newWidth = (int) (src.size().width / hRatio);
	}

	resize(src, resized, Size(newWidth, newHeight));
	return resized;
}

Point computeCentralPoint(vector<Point> contours) {
	int x = 0;
	int y = 0;

	for (int i = 0; i < contours.size(); i++) {
		x += contours[i].x;
		y += contours[i].y;
	}

	Point center = Point((int) (x / contours.size()), y / (int) (contours.size()));

	return center;
}


vector<Point2f> getCardCorners(Point central, vector<Point> contours, int mode) {

	//Distance parameter
	int par = 70;
	if (mode == 2) {
		RotatedRect rotatedRect = minAreaRect(contours);
		Point2f rect_points[4];

		// gets the corner points of each card
		rotatedRect.points(rect_points);
		vector<Point2f> v(rect_points, rect_points + sizeof rect_points / sizeof rect_points[0]);
		return v;
		
	}
	//Maps a point and its distance to central point
	map<PointOrder*, float> distanceMap;
	map<PointOrder*, float>::iterator itMap;
	float distance = 0;

	//Computes all distances from all contours to the center point
	for (unsigned int i = 0; i < contours.size(); i++) {
		distance = distancePoints(central, contours[i]);
		distanceMap.insert(pair<PointOrder*, float>(new PointOrder(&contours[i], i), distance));
	}

	vector<Point2f> cornerPoints;
	vector<PointOrder> unorderedPoints;
	while (unorderedPoints.size() < 4) {
		pair<PointOrder*, float> max = *max_element(distanceMap.begin(), distanceMap.end(), pairCompare);

		if (unorderedPoints.size() == 0) {
			unorderedPoints.push_back(*max.first);
			itMap = distanceMap.find(max.first);
			distanceMap.erase(itMap);
		}
		else {
			bool insertPoint = true;
			for (unsigned int i = 0; i < unorderedPoints.size(); i++) {
				if (distancePoints((*max.first->point), (*unorderedPoints[i].point)) < par) {
					insertPoint = false;
					break;
				}
				
			}
			if (insertPoint)
				unorderedPoints.push_back(*max.first);

			//Remove the already processed point
			itMap = distanceMap.find(max.first);
			distanceMap.erase(itMap);

		}

		
	}

	// Order the points
	sort(unorderedPoints.begin(), unorderedPoints.end(), compareIndex);

	for (unsigned int i = 0; i < unorderedPoints.size(); i++) {
		cornerPoints.push_back(*unorderedPoints[i].point);
	}

	return cornerPoints;

}

bool pairCompare(pair<PointOrder*, float> p, pair<PointOrder*, float> p1) {
	return p.second < p1.second;
}

void showLoadingBar(int progress) {
	if (progress >= 99)
		progress = 100;
	cout << "\r";
	cout << "[";
	for (int j = 0; j < progress / 2; j++) {
		cout << "=";
	}
	cout << ">] " << progress << "%";
}