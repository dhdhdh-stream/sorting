/**
 * TODO: can be real, have multiple components, etc.
 */

#ifndef ACTION_H
#define ACTION_H

#include <fstream>
#include <string>

class Action {
public:
	int move;

	Action();
	Action(int move);
	Action(std::ifstream& save_file);

	bool operator==(const Action& rhs);

	void save(std::ofstream& save_file);
};

#endif /* ACTION_H */