#pragma once

#include <string>
#include "Opencv.h"

class Card {
private:
	std::string name;
	std::string suit;
	int score;
	cv::Mat img;
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
public:
	Card();
	Card(std::string, std::string, int);
	Card(std::string, std::string ,int, cv::Mat);
	std::string getCard();
	std::string getSuit();
	int getScore();
	cv::Mat getCardImg();
	void setImg(cv::Mat img);
	std::vector<cv::KeyPoint> getKeypoints();
	cv::Mat getDescriptors();
	void computeKeypoints();
	void computeDescriptors();
	~Card();
};

