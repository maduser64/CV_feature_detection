#ifndef CARDS_DETECTION_H_
#define CARDS_DETECTION_H_

#include <iostream>
#include <string>
#include "Opencv.h"

extern int MAX_KERNEL_SIZE;

/* Road detection using uploaded images (.bmp, .jpg, .JPEG, .png, ...) */
void imageBasedVersion(std::string);

/* Road detection using video */
void videoBasedVersion();

/* Compares areas */
bool compareAreas(std::vector<cv::Point>, std::vector<cv::Point>);

#endif /* CARDS_DETECTION_H_ */