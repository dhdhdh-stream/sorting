#ifndef RAW_ACTION_H
#define RAW_ACTION_H

#include <fstream>
#include <string>

const int LEFT = 0;
const int STAY = 1;
const int RIGHT = 2;

class RawAction {
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

#endif /* RAW_ACTION_H */