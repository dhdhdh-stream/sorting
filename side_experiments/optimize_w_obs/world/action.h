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

	Action();
	Action(int move);
	Action(std::ifstream& save_file);
	~Action();

	bool operator==(const Action& rhs);

	void save(std::ofstream& save_file);
};

#endif /* ACTION_H */