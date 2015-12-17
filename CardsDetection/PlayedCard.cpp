#include "PlayedCard.h"

using namespace std;
using namespace cv;

PlayedCard::PlayedCard () {
	this->originalImg = NULL;
	this->rotatedImg = NULL;
	this->leastDifferentCard = NULL;
	this->differences.clear();
}

PlayedCard::PlayedCard(Mat originalImg, Mat rotatedImg, vector<Point> contours, vector<Point2f> cornerPoints, vector<Card*> deck) {
	this->originalImg = originalImg;
	this->rotatedImg = rotatedImg;
	this->leastDifferentCard = NULL;
	this->cornerPoints = cornerPoints;
	this->contours = contours;

	// Compute the central point using contours
	computeCentralPoint();
	// Compute the difference between all deck cards
	computeAbsDifference(deck);
}

Mat PlayedCard::getOriginalImg() {
	return originalImg;
}

Mat PlayedCard::getRotatedImg() {
	return rotatedImg;
}

void PlayedCard::setRotatedImg(cv::Mat img) {
	this->rotatedImg = img;
}

map<Card*, int> PlayedCard::getCardDifferences() {
	return differences;
}

bool pairCompare(pair<Card*, int> p, pair<Card*, int> p1) {
	return p.second < p1.second;
}

std::vector<cv::Point2f> PlayedCard::getCornerPoints() {
	return this->cornerPoints;
}


void PlayedCard::computeAbsDifference(vector<Card*> deck) {
	for (int i = 0; i < deck.size(); i++) {
		Mat diff;
		Mat diffRotated;

		absdiff(this->originalImg, deck[i]->getCardImg(), diff);
		absdiff(this->rotatedImg, deck[i]->getCardImg(), diffRotated);


		GaussianBlur(diff, diff, Size(5, 5), 5);
		threshold(diff, diff, 200, 255, CV_THRESH_BINARY);

		GaussianBlur(diffRotated, diffRotated, Size(5, 5), 5);
		threshold(diffRotated, diffRotated, 200, 255, CV_THRESH_BINARY);

		Scalar sOriginal(sum(diff));
		Scalar sRotated(sum(diffRotated));

		int diffValue = ((int) sOriginal[0] + (int) sRotated[0]) / 2;
		
		differences.insert(pair<Card*, int>(deck[i], diffValue));
	}
	
	pair<Card*, int> min = *min_element(differences.begin(), differences.end(), pairCompare);
	this->leastDifferentCard = min.first;
}

Card* PlayedCard::getLeastDifferentCard() {
	return leastDifferentCard;
}

void PlayedCard::computeCentralPoint() {
	int x = 0;
	int y = 0;
	for (int i = 0; i < this->contours.size(); i++) {
		x += contours[i].x;
		y += contours[i].y;
	}

	Point center = Point(x / this->contours.size(), y / this->contours.size());

	this->centralPoint = center;
}

Point PlayedCard::getCentralPoint(){
	return this->centralPoint;
}

PlayedCard::~PlayedCard() 
{}