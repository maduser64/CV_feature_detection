#pragma once

#include <string>
#include "Opencv.h"

class Card {
private:
	std::string name;
	std::string suit;
	int score;
	cv::Mat img;
public:
	Card(std::string, std::string ,int, cv::Mat);
	~Card();
};

