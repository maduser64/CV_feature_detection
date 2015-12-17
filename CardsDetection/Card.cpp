#include "Card.h"

using namespace std;
using namespace cv;

Card::Card() {
	this->name = "";
	this->suit = "";
	this->score = -1;
	this->img = NULL;
}

Card::Card(string name, string suit, int score) {
	this->name = name;
	this->suit = suit;
	this->score = score;
	this->img = NULL;
}

Card::Card(string name, string suit, int score, Mat img) {
	this->name = name;
	this->suit = suit;
	this->score = score;
	this->img = img;
}

Card::~Card()
{}

string Card::getCard() {
	return name;
}

string Card::getSuit() {
	return suit;
}

int Card::getScore() {
	return score;
}

Mat Card::getCardImg() {
	return img;
}

void Card::setImg(Mat img) {
	this->img = img;
}

vector<KeyPoint> Card::getKeypoints() {
	return keypoints;
}
cv::Mat Card::getDescriptors() {
	return descriptors;
}

void Card::computeKeypoints() {
	int minHessian = 1000;
	SurfFeatureDetector detector(minHessian);
	detector.detect(img, keypoints);
}

void Card::computeDescriptors() {
	SurfDescriptorExtractor extractor;
	extractor.compute(img, keypoints, descriptors);
}