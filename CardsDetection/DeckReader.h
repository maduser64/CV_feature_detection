#ifndef DECK_READER_H_
#define DECK_READER_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include "Card.h"
#include "Opencv.h"

bool deckPreProcessed();

std::vector<Card*> getDeck();

std::vector<Card*> readDeckFile();

int getCardScore(std::string);

inline bool fileExists(const std::string& name);

#endif /* DECK_READER_H_ */