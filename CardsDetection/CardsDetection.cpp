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
			//progressBar(i, 54, 50, 100);
			cout << "coco" << endl;
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

void imageBasedVersion(string imagesDir, int mode) {

	Mat img = imread(imagesDir, CV_LOAD_IMAGE_COLOR);
	//img = resizeImage(img, Size(1000, 700));
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
	vector<vector<Point2f>> cornerPoints;

	Point2f outputQuad[4] = { Point2f(0, 0), Point2f(CARD_X_SIZE - 1, 0), Point2f(CARD_X_SIZE - 1, CARD_Y_SIZE - 1), Point2f(0, CARD_Y_SIZE - 1) };

	if (contours.size() >= 4) {
		Scalar color = Scalar(0, 255, 0);
		// draw the first 4 contours, this will hopefuly represent the 4 cards
		for (unsigned int i = 0; i < 4; i++) {

			

			for (int j = 0; j < contours[i].size(); j++) {
				//cout << contours[i][j].x << " | " << contours[i][j].y << endl;
				// Draw contour points
				circle(img, (Point) contours[i][j], 1, Scalar(255, 0, 0), 5);
			}
			


			// find card center points
			centerPoints[i] = computeCentralPoint(contours[i]);

			circle(img, (Point) centerPoints[i], 1, Scalar(255, 0, 255), 5);


			vector<Point> cornersNew = getCardCorners(centerPoints[i], contours[i]);

			for (int j = 0; j < cornersNew.size(); j++) {
				// Draw contour points
				//circle(img, cornersNew[j], 1, Scalar(255, 255, 0), 5);
			}

			// Draw contour points
			circle(img, centerPoints[i], 1, Scalar(255, 255, 0), 5);
			

			//drawContours(img, contours, i, color, 4, 8, hierarchy, 0);
			RotatedRect rotatedRect = minAreaRect(contours[i]);
			Point2f rect_points[4];

			// gets the corner points of each card
			rotatedRect.points(rect_points);

			// Rotates card to make it vertical
			cardToVertical(rect_points);
			
			// inserts it on a vector for later use, on card class construction
			vector<Point2f> tempPoints;
			for (unsigned int k = 0; k < 4; k++)
				tempPoints.push_back(rect_points[k]);

			cornerPoints.push_back(tempPoints);

			for (int j = 0; j < cornerPoints[i].size(); j++) {
				//cout << contours[i][j].x << " | " << contours[i][j].y << endl;
				// Draw contour points
				circle(img, (Point) cornerPoints[i][j], 1, Scalar(255, 0, 255), 5);
			}
			
			// Calculates a perspective transform from four pairs of the corresponding points
			lambda = getPerspectiveTransform(rect_points, outputQuad);
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
		playedCards.push_back(new PlayedCard(procCards[i], rotatedCard, contours[i], cornerPoints[i], deck, mode, i));
	}

	getWinner(playedCards);

	// Show matched cards info
	for (unsigned int i = 0; i < 4; i++) {
		string card = playedCards[i]->getLeastDifferentCard()->getCard();
		string suit = playedCards[i]->getLeastDifferentCard()->getSuit();
		int score = playedCards[i]->getLeastDifferentCard()->getScore();
		cout << "Card: " << card << " | suit: " << suit << " | score: " << score << endl;

		// write info about cards
		playedCards[i]->drawCardText(img);
		
	}

	namedWindow("Image", WINDOW_AUTOSIZE);
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

		if (waitKey(30) == 32) {
			cout << "Image saved" << endl;
			image = frame;
			state = 1;
			break;
		}
	}

	destroyWindow("Webcam");
	if (state == 1)
		processVideo(image, mode);
}

void processVideo(Mat frame, int mode) {
	
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

	imageBasedVersion(fileName, mode);

	waitKey(0);

	if (continueWebcam()) {
		destroyAllWindows();
		videoBasedVersion(mode);
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

	cout << "contours size " << contours.size() << endl;
	for (int i = 0; i < contours.size(); i++) {
		x += contours[i].x;
		y += contours[i].y;
	}

	Point center = Point((int) (x / contours.size()), y / (int) (contours.size()));

	return center;
}

vector<Point> getCardCorners(Point central, vector<Point> contours) {

	//Maps a point and its distance to central point
	map<Point*, float> distanceMap;
	map<Point*, float>::iterator itMap;
	float distance = 0;

	//Computes all distances from all contours to the center point
	for (unsigned int i = 0; i < contours.size(); i++) {
		distance = distancePoints(central, contours[i]);
		distanceMap.insert(pair<Point*, float>(&contours[i], distance));
	}

	vector<Point> cornerPoints;
	while (cornerPoints.size() < 4) {
		pair<Point*, float> max = *max_element(distanceMap.begin(), distanceMap.end(), pairCompare);

		if (cornerPoints.size() == 0) {
			cornerPoints.push_back(*max.first);
			itMap = distanceMap.find(max.first);
			distanceMap.erase(itMap);
		}
		else {
			bool insertPoint = true;
			for (unsigned int i = 0; i < cornerPoints.size(); i++) {
				if (distancePoints(*max.first, cornerPoints[i]) < 105) {
					insertPoint = false;
					break;
				}
			}

			if (insertPoint)
				cornerPoints.push_back(*max.first);

			//Remove the already processed point
			itMap = distanceMap.find(max.first);
			distanceMap.erase(itMap);

		}
		
	}
	
	return cornerPoints;

}

bool pairCompare(pair<Point*, float> p, pair<Point*, float> p1) {
	return p.second < p1.second;
}

void progressBar(int x, int n, int r, int w) {
	// Only update r times.
	if (x % (n / r + 1) != 0) return;

	// Calculuate the ratio of complete-to-incomplete.
	float ratio = x / (float) n;
	int   c = ratio * w;

	// Show the percentage complete.
	printf("%3d%% [", (int) (ratio * 100));

	// Show the load bar.
	for (int x = 0; x<c; x++)
		printf("=");

	for (int x = c; x<w; x++)
		printf(" ");

	// ANSI Control codes to go back to the
	// previous line and clear it.
	printf("]\n\033[F\033[J");
}