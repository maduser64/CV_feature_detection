#include "PlayedCard.h"

using namespace std;
using namespace cv;

PlayedCard::PlayedCard () {
	this->originalImg = NULL;
	this->rotatedImg = NULL;
	this->differences.clear();
}

PlayedCard::PlayedCard(Mat originalImg, Mat rotatedImg) {
	this->originalImg = originalImg;
	this->rotatedImg = rotatedImg;
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

PlayedCard::~PlayedCard() 
{}