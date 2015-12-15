#include "DeckReader.h"

using namespace std;
using namespace cv;

bool deckPreProcessed() {
	string filePath = "deck/deck.txt";
	if (fileExists(filePath))
		return true;
	else
		return false;
}

inline bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

vector<Card*> readDeckFile() {
	vector <Card*> deck; deck.clear();
	
	size_t cardSize = -1;
	string line = "";
	ifstream infile("deck/deck.txt");
	while (getline(infile, line)) {
		cardSize = line.size();
	
		int score = -1;
		string card = "";
		string suit = "";
		switch (cardSize) {
		case 2:
			card = line.substr(0, 1);
			suit = line.substr(1, 1);
			score = getCardScore(card);
			break;
		case 3:
			card = line.substr(0, 2);
			suit = line.substr(2, 1);
			score = getCardScore(card);
			break;
		case 5:
			card = line;
			suit = "none";
			score = getCardScore(card);
			break;
		default:
			break;
		}

		deck.push_back(new Card(card, suit, score));
	}
	return deck;
}

vector<Card*> getDeck() {
	vector<Card*> deck = readDeckFile();

	int x = 0;
	int y = 0;
	string deckPath = "deck/deck_array.jpg"; 
	Mat deckImg = imread(deckPath, CV_LOAD_IMAGE_COLOR);
	for (int i = 0; i < deck.size(); i++) {
		y = 0;
		x = 450 * i;
		Rect cardROI(x, y, 450, 450);
		deck[i]->setImg(Mat(deckImg(cardROI)));
	}
	return deck;
}

int getCardScore(string card) {
	if (card == "2")
		return 0;
	else if (card == "3")
		return 0;
	else if (card == "4")
		return 0;
	else if (card == "5")
		return 0;
	else if (card == "6")
		return 0;
	else if (card == "7")
		return 0;
	else if (card == "8")
		return 0;
	else if (card == "9")
		return 0;
	else if (card == "10")
		return 0;
	else if (card == "Q")
		return 0;
	else if (card == "J")
		return 0;
	else if (card == "K")
		return 0;
	else if (card == "A")
		return 0;
	else if (card == "Joker")
		return 0;
	else
		return -1;
}
