#ifndef STATE_H
#define STATE_H

#include <fstream>
#include <set>
#include <vector>

class AbstractNode;
class FullNetwork;
class Scope;

class State {
public:
	int id;

	std::vector<FullNetwork*> networks;

	State();
	State(std::ifstream& input_file,
		  int id);
	~State();

	void save(std::ofstream& output_file,
			  std::string name);
};

#endif /* STATE_H */