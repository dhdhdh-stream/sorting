#ifndef ACTION_H
#define ACTION_H

#include <fstream>
#include <string>

const int LEFT = 0;
const int STAY = 1;
const int RIGHT = 2;

const int COMPOUND = 3;
const int LOOP = 4;

class Action {
public:
	double write;
	int move;

	int index;

	Action(double write, int move);
	Action(int move, int index);
	Action(std::ifstream& save_file);
	~Action();

	void save(std::ofstream& save_file);

	std::string to_string();
};

const Action HALT(0.0, -1);
const Action EMPTY_ACTION(0.0, -1);

#endif /* ACTION_H */