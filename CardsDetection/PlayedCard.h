#pragma once

#include <string>
#include <map>
#include "Card.h"
#include "Opencv.h"

class PlayedCard {
private:
	cv::Mat originalImg;
	cv::Mat rotatedImg;
	std::map <Card*, int> differences;
	Card* leastDifferentCard;
	std::vector < cv::Point2f > cornerPoints;
public:
	/*Default constructor*/
	PlayedCard();
	/*Receives the original card, rotated card and the deck array*/
	PlayedCard(cv::Mat, cv::Mat, std::vector<cv::Point2f>, std::vector<Card*>);
	/*Returns the unrotated card*/
	cv::Mat getOriginalImg();
	/*Returns the rotated card*/
	cv::Mat getRotatedImg();
	/*Returns the most similar card found*/
	Card* getLeastDifferentCard();
	/*Returns the cornerPoints*/
	std::vector<cv::Point2f> getCornerPoints();
	/*Sets a new rotated card*/
	void setRotatedImg(cv::Mat img);
	/*Returns a map that holds the difference value between each deck card*/
	std::map<Card*, int> getCardDifferences();
	/*Computer the absolute difference, mean between the sum of the difference of both rotated and unrotated card*/
	void computeAbsDifference(std::vector<Card*>);

	/*Computer the difference using SURF*/
	void computeDifferenceSurf(std::vector<Card*>);
	int computeSurfGoodMatches(cv::Mat, cv::Mat);
	void filterMatchesByAbsoluteValue(std::vector<cv::DMatch>&, float);
	cv::Mat filterMatchesRANSAC(std::vector<cv::DMatch>&, std::vector<cv::KeyPoint>&, std::vector<cv::KeyPoint>&);
	~PlayedCard();
};