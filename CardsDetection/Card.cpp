#include "Card.h"

using namespace std;
using namespace cv;

Card::Card(string name, string suit, int score, Mat img){
	this->name = name;
	this->suit = suit;
	this->score = score;
	this->img = img;
}


Card::~Card()
{
}
