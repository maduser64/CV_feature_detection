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

bool deckPreProcessed(int);

std::vector<Card*> getDeck(int);

std::vector<Card*> readDeckFile();

int getCardScore(std::string);

inline bool fileExists(const std::string&);

#endif /* DECK_READER_H_ */