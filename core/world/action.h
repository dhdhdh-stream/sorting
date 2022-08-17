#ifndef ACTION_H
#define ACTION_H

#include <fstream>
#include <string>

const int LEFT = 0;
const int STAY = 1;
const int RIGHT = 2;

class Action {
public:
	double write;
	int move;

	Action(double write, int move);
	Action(std::ifstream& save_file);
	~Action();

	void save(std::ofstream& save_file);

	std::string to_string();
};

#endif /* ACTION_H */