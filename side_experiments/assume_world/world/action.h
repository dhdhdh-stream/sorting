/**
 * TODO: can be real, have multiple components, etc.
 */

#ifndef ACTION_H
#define ACTION_H

#include <fstream>
#include <string>

const int ACTION_NOOP = -1;

class Action {
public:
	int move;
	int distance;

	Action();
	Action(int move, int distance);
	Action(std::ifstream& save_file);
	~Action();

	void save(std::ofstream& save_file);
};

#endif /* ACTION_H */