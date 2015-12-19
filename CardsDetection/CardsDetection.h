#ifndef CARDS_DETECTION_H_
#define CARDS_DETECTION_H_

#include <iostream>
#include <string>
#include <windows.h>
#include "DeckReader.h"
#include "PlayedCard.h"
#include "Opencv.h"
#include "Cli.h"
#include "time.h"

extern int MAX_KERNEL_SIZE;


class PointOrder {
public:
	cv::Point* point;
	int order;
	PointOrder(cv::Point* point, int order) { this->point = point; this->order = order; }
};

/* Receives a point array and transforms the card mapping to vertical */
void cardToVertical(cv::Point2f*);
/*Rotates a given image with received degrees*/
void rotateCard(cv::Mat&, double, cv::Mat&);
/*Rotates a given received text material*/
void rotateCard(cv::Mat&, double, cv::Mat&);
/* Returns the distance between 2 points */
float distancePoints(cv::Point2f, cv::Point2f);
/*Aux function to processDeck function, appends a card by card on array of cards*/
cv::Mat appendImages(cv::Mat, cv::Mat, int);
/* Receives one image with entire deck and saves an array with cards */
void processDeck(std::string, int);
/*Compares cards and returns the absolute difference between them*/
int getAbsDifference(cv::Mat, cv::Mat);
/* Road detection using uploaded images (.bmp, .jpg, .JPEG, .png, ...) */
void imageBasedVersion(std::string, int, int);
/* Road detection using video */
void videoBasedVersion(int, int);
/* Process video frame */
void processVideo(cv::Mat, int, int);
/*receives v1 and v2 point vectors and if v1 greater v2 return true*/
bool compareAreas(std::vector<cv::Point>, std::vector<cv::Point>);
bool compareAreasF(std::vector<cv::Point2f>, std::vector<cv::Point2f>);
/*Returns the card with highest score*/
void getWinner(std::vector<PlayedCard*>);
/*Resizes a image for a given size*/
cv::Mat resizeImage(cv::Mat, cv::Size);
/*Shows a progress bar*/
void progressBar(int, int, int, int);
/*Computes a card central point based on contours points*/
cv::Point computeCentralPoint(std::vector<cv::Point>);
/**/
std::vector<cv::Point2f> getCardCorners(cv::Point, std::vector<cv::Point>, int);
/*Draw loading bar*/
void showLoadingBar(int progress);
bool pairCompare(std::pair<PointOrder*, float>, std::pair<PointOrder*, float>);
#endif /* CARDS_DETECTION_H_ */