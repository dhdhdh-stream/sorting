#ifndef ACTION_H
#define ACTION_H

#include <fstream>
#include <string>

const int ACTION_NOOP = -1;

const int ACTION_LEFT = 0;
const int ACTION_RIGHT = 1;
const int ACTION_SWAP = 2;

class Action {
public:
	int move;

	Action();
	Action(int move);
	Action(std::ifstream& save_file);
	~Action();

	void save(std::ofstream& save_file);

	std::string to_string();
};

#endif /* ACTION_H */