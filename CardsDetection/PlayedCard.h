#pragma once

#include <string>
#include <map>
#include "Card.h"
#include "Opencv.h"

class PlayedCard {
private:
	cv::Mat originalImg;
	cv::Mat rotatedImg;
	cv::Point centralPoint;
	/*Winner->w | Looser->l | Draw->d*/
	char result;
	
	std::map <Card*, int> differences;
	Card* leastDifferentCard;
	std::vector < cv::Point2f > cornerPoints;
	std::vector<cv::KeyPoint> keypointsOriginal;
	std::vector<cv::KeyPoint> keypointsRotated;
	cv::Mat descriptorsOriginal;
	cv::Mat descriptorsRotated;
	std::vector < cv::Point > contours;
public:
	/*Default constructor*/
	PlayedCard();
	/*Receives the original card, rotated card and the deck array // mode = 0 -> Use subtraction / mode = 1 -> Use surf */
	PlayedCard(cv::Mat, cv::Mat, std::vector<cv::Point>, std::vector<cv::Point2f>, std::vector<Card*>, int);
	/*Returns the unrotated card*/
	cv::Mat getOriginalImg();
	/*Returns the rotated card*/
	cv::Mat getRotatedImg();
	/*Returns the most similar card found*/
	Card* getLeastDifferentCard();
	/*Returns the cornerPoints*/
	std::vector<cv::Point2f> getCornerPoints();
	/*Returns the contours*/
	std::vector<cv::Point2f> getContours();
	/*Computes a card's central point*/
	void computeCentralPoint();
	/*returns the central point*/
	cv::Point getCentralPoint();
	/*Sets a new rotated card*/
	void setRotatedImg(cv::Mat);
	/*Sets the winner state*/
	void setWinner(char);
	/*Gets the winner state*/
	char getWinner();
	/*Returns a map that holds the difference value between each deck card*/
	std::map<Card*, int> getCardDifferences();
	/*Computes the absolute difference, mean between the sum of the difference of both rotated and unrotated card*/
	void computeAbsDifference(std::vector<Card*>);
	/*Draws the text on the card*/
	void drawCardText(cv::Mat&);
	
	std::vector<cv::KeyPoint> getKeypointsOriginal();
	cv::Mat getDescriptorsOriginal();
	std::vector<cv::KeyPoint> getKeypointsRotated();
	cv::Mat getDescriptorsRotated();
	void computeKeypoints();
	void computeDescriptors();

	/*Computer the difference using SURF*/
	void computeDifferenceSurf(std::vector<Card*>);
	int computeSurfGoodMatches(std::vector<cv::KeyPoint>, std::vector<cv::KeyPoint>, cv::Mat, cv::Mat);
	void filterMatchesByAbsoluteValue(std::vector<cv::DMatch>&, float);
	cv::Mat filterMatchesRANSAC(std::vector<cv::DMatch>&, std::vector<cv::KeyPoint>&, std::vector<cv::KeyPoint>&);

	~PlayedCard();
};