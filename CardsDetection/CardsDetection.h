#ifndef CARDS_DETECTION_H_
#define CARDS_DETECTION_H_

#include <iostream>
#include <string>
#include "DeckReader.h"
#include "PlayedCard.h"
#include "Opencv.h"

extern int MAX_KERNEL_SIZE;

/* Receives a point array and transforms the card mapping to vertical */
void cardToVertical(cv::Point2f*);

/*Rotates a given image with received degrees*/
void rotateCard(cv::Mat&, double angle, cv::Mat& dst);

/* Returns the distance between 2 points */
float distancePoints(cv::Point2f p1, cv::Point2f p2);

/*Aux function to processDeck function, appends a card by card on array of cards*/
cv::Mat appendImages(cv::Mat deckArray, cv::Mat card, int index);

/* Receives one image with entire deck and saves an array with cards */
void processDeck(std::string imagesDir, int mode);

/*Compares cards and returns the absolute difference between them*/
int getAbsDifference(cv::Mat img1, cv::Mat img2);

/* Road detection using uploaded images (.bmp, .jpg, .JPEG, .png, ...) */
void imageBasedVersion(std::string, int mode);

/* Road detection using video */
void videoBasedVersion();

/* Compares areas */
bool compareAreas(std::vector<cv::Point>, std::vector<cv::Point>);

/*Returns the card with highest score*/
PlayedCard* getWinner(std::vector<PlayedCard*>);

/*Returns a card's center point*/
cv::Point getCenterPoint(PlayedCard*);

#endif /* CARDS_DETECTION_H_ */