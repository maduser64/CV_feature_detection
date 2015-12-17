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
	std::map <Card*, int> differences;
	Card* leastDifferentCard;
	std::vector < cv::Point2f > cornerPoints;
	std::vector < cv::Point > contours;
public:
	/*Default constructor*/
	PlayedCard();
	/*Receives the original card, rotated card and the deck array*/
	PlayedCard(cv::Mat, cv::Mat, std::vector<cv::Point>, std::vector<cv::Point2f>, std::vector<Card*>);
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
	/*Sets a new rotated card*/
	void setRotatedImg(cv::Mat img);
	/*Returns a map that holds the difference value between each deck card*/
	std::map<Card*, int> getCardDifferences();
	/*Computes the absolute difference, mean between the sum of the difference of both rotated and unrotated card*/
	void computeAbsDifference(std::vector<Card*>);
	/*Computes a card's central point*/
	void computeCentralPoint();
	/*returns the central point*/
	cv::Point getCentralPoint();
	~PlayedCard();
};