#include "PlayedCard.h"

using namespace std;
using namespace cv;

int computeSurfGoodMatches(Mat, Mat);

PlayedCard::PlayedCard () {
	this->originalImg = NULL;
	this->rotatedImg = NULL;
	this->leastDifferentCard = NULL;
	this->differences.clear();
}

// mode = 0 -> Use subtraction / mode = 1 -> Use surf
PlayedCard::PlayedCard(Mat originalImg, Mat rotatedImg, vector<Point> contours, vector<Point2f> cornerPoints, vector<Card*> deck, int mode) {
	this->originalImg = originalImg;
	this->rotatedImg = rotatedImg;
	this->leastDifferentCard = NULL;
	this->cornerPoints = cornerPoints;
	this->contours = contours;

	computeCentralPoint();
	// Compute the difference with all deck cards
	if (mode == 0)
		computeAbsDifference(deck);
	else if (mode == 1) {
		computeKeypoints();
		computeDescriptors();
		computeDifferenceSurf(deck);
	}
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

vector<KeyPoint> PlayedCard::getKeypointsOriginal() {
	return keypointsOriginal;
}

cv::Mat PlayedCard::getDescriptorsOriginal() {
	return descriptorsOriginal;
}

vector<KeyPoint> PlayedCard::getKeypointsRotated() {
	return keypointsRotated;
}

cv::Mat PlayedCard::getDescriptorsRotated() {
	return descriptorsRotated;
}

void PlayedCard::computeKeypoints() {
	int minHessian = 1000;
	SurfFeatureDetector detector(minHessian);
	detector.detect(originalImg, keypointsOriginal);
	detector.detect(rotatedImg, keypointsRotated);
}

void PlayedCard::computeDescriptors() {
	SurfDescriptorExtractor extractor;
	extractor.compute(originalImg, keypointsOriginal, descriptorsOriginal);
	extractor.compute(rotatedImg, keypointsRotated, descriptorsRotated);
}

void PlayedCard::computeDifferenceSurf(vector<Card*> deck) {

	for (int i = 0; i < deck.size(); i++) {
		int goodMatchesOriginal = computeSurfGoodMatches(keypointsOriginal, deck[i]->getKeypoints(), descriptorsOriginal, deck[i]->getDescriptors());
		int goodMatchesRotated = computeSurfGoodMatches(keypointsRotated, deck[i]->getKeypoints(), descriptorsRotated, deck[i]->getDescriptors());

		int matches = goodMatchesOriginal + goodMatchesRotated;
		differences.insert(pair<Card*, int>(deck[i], matches));
	}

	pair<Card*, int> max = *max_element(differences.begin(), differences.end(), pairCompare);
	this->leastDifferentCard = max.first;
	printf("Match: %s %s \n", this->leastDifferentCard->getCard(), this->leastDifferentCard->getSuit());
}

int PlayedCard::computeSurfGoodMatches(vector<KeyPoint> keypoints_1, vector<KeyPoint> keypoints_2, Mat descriptors_1, Mat descriptors_2) {

	//-- Step 3: Matching descriptor vectors using FLANN matcher
	FlannBasedMatcher matcher;
	vector< DMatch > matches;
	matcher.match(descriptors_1, descriptors_2, matches);

	filterMatchesByAbsoluteValue(matches, 0.125);
	filterMatchesRANSAC(matches, keypoints_1, keypoints_2);

	//Mat img_matches;
	//drawMatches(img, keypoints_1, img1, keypoints_2, matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	//imshow("matches", img_matches);
	//printf("Good Matches: %d \n", (int)matches.size());
	//waitKey(0);

	return (int)matches.size();
}

void PlayedCard::filterMatchesByAbsoluteValue(vector<DMatch> &matches, float maxDistance) {
	vector<DMatch> filteredMatches;
	for (size_t i = 0; i<matches.size(); i++)
	{
		if (matches[i].distance < maxDistance)
			filteredMatches.push_back(matches[i]);
	}
	matches = filteredMatches;
}

Mat PlayedCard::filterMatchesRANSAC(vector<DMatch> &matches, vector<KeyPoint> &keypointsA, vector<KeyPoint> &keypointsB) {
	Mat homography;
	std::vector<DMatch> filteredMatches;
	if (matches.size() >= 4)
	{
		vector<Point2f> srcPoints;
		vector<Point2f> dstPoints;
		for (size_t i = 0; i<matches.size(); i++)
		{

			srcPoints.push_back(keypointsA[matches[i].queryIdx].pt);
			dstPoints.push_back(keypointsB[matches[i].trainIdx].pt);
		}

		Mat mask;
		homography = findHomography(srcPoints, dstPoints, CV_RANSAC, 1.0, mask);

		for (int i = 0; i<mask.rows; i++)
		{
			if (mask.ptr<uchar>(i)[0] == 1)
				filteredMatches.push_back(matches[i]);
		}
	}
	matches = filteredMatches;
	return homography;
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

	Point center = Point( (int) (x / this->contours.size()), y / (int) (this->contours.size()));

	this->centralPoint = center;
}

void PlayedCard::drawCardText(cv::Mat &srcImg) {

	int FONT_FACE = FONT_HERSHEY_TRIPLEX;
	int FONT_SCALE = 3;
	int FONT_THICKNESS = 5;
	int TOP_MARGIN = 80;

	Mat emptyCard = Mat::zeros(450, 450, srcImg.type());
	Mat emptyImg = Mat::zeros(srcImg.size(), srcImg.type());

	/*********************************************Top center text*********************************************/
	
	string cardName = this->getLeastDifferentCard()->getCard() + " " + this->getLeastDifferentCard()->getSuit();
	

	Size cardNameSize = getTextSize(cardName, FONT_FACE, FONT_SCALE, FONT_THICKNESS, 0);

	Point topCenterPoint = Point(225 - cardNameSize.width / 2, TOP_MARGIN);

	putText(emptyCard, cardName, topCenterPoint, FONT_FACE, FONT_SCALE, Scalar(255, 255, 255), FONT_THICKNESS);

	/***************************************************************************************************/

	/*********************************************Center text*********************************************/
	
	string result;
	Scalar color;
	if (this->result == 'w') {
		result = "Winner";
		color = Scalar(0, 255, 0);
	}
	else if(this->result == 'l') { 
		result = "Looser"; 
		color = Scalar(0, 0, 255);
	}
	else if (this->result == 'd') {
		result = "Draw";
		color = Scalar(255, 255,  0);
	}

	Size resultSize = getTextSize(result, FONT_FACE, FONT_SCALE, FONT_THICKNESS, 0);

	Point centerPoint = Point(225 - resultSize.width / 2, 225 + resultSize.height / 2);


	putText(emptyCard, result, centerPoint, FONT_FACE, FONT_SCALE, color, FONT_THICKNESS);

	/***************************************************************************************************/

	
	Point2f outputQuad[4] = { Point2f(0, 0), Point2f(450 - 1, 0), Point2f(450 - 1, 450 - 1), Point2f(0, 450 - 1) };
	Point2f cornerArray[4];

	for (size_t i = 0; i < 4; i++) {
		cornerArray[i] = this->cornerPoints[i];
	}

	Mat lambda = getPerspectiveTransform(outputQuad, cornerArray);
	warpPerspective(emptyCard, emptyImg, lambda, srcImg.size());


	srcImg += emptyImg;
}

void PlayedCard::setWinner(char result){
	this->result = result;
}

char PlayedCard::getWinner(){
	return this->result;
}

Point PlayedCard::getCentralPoint(){
	return this->centralPoint;
}

PlayedCard::~PlayedCard() 
{}