#include "Cli.h"

using namespace std;

int initCli() {

	int choice = -1;
	cout << "Welcome to the road detection program!" << endl;

	while (true) {
		cout << "Please select the mode in which you want to execute the program" << endl;
		cout << "1 -> Upload an image" << endl;
		cout << "2 -> Use computer's webcam" << endl;
		cout << ">> ";
		cin >> choice;

		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Invalid entry" << endl;
		}
		else {
			if (choice < 1 || choice > 2) {
				cin.clear();
				cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
				cout << "You have inserted an ivalid option" << endl;
			}
			else
				break;
		}
	}
	return choice;
}

string getImgPath(string dir) {
	string imgPath = "";
	string filePath = "";
	while (true) {
		cout << "Please insert the file name" << endl;
		cout << ">> ";
		cin >> imgPath;
		filePath = dir + imgPath;
		if (fileExists(filePath))
			break;
		else
			cout << "The name you specified does not exist, please try again " << filePath << endl;
	}
	return filePath;
}

inline bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}