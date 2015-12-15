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
public:
	PlayedCard();
	PlayedCard(cv::Mat, cv::Mat);
	cv::Mat getOriginalImg();
	cv::Mat getRotatedImg();
	void setRotatedImg(cv::Mat img);
	std::map<Card*, int> getCardDifferences();
	void PlayedCard::computeAbsDifference(std::vector<Card*>);
	~PlayedCard();
};