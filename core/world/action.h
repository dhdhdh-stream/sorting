#ifndef ACTION_H
#define ACTION_H

#include <fstream>
#include <string>

const int ACTION_START = -1;

const int ACTION_LEFT = 0;
const int ACTION_STAY = 1;
const int ACTION_RIGHT = 2;

class Action {
public:
	double write;
	int move;

	Action();	// empty constructor for convenience
	Action(double write, int move);
	Action(std::ifstream& save_file);
	~Action();

	void save(std::ofstream& save_file);

	std::string to_string();
};

#endif /* ACTION_H */