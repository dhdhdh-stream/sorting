#ifndef INPUT_H
#define INPUT_H

#include <fstream>
#include <vector>

class Scope;
class Solution;

const int INPUT_TYPE_REMOVED = -1;
const int INPUT_TYPE_OBS = 0;
const int INPUT_TYPE_INPUT = 1;

class Input {
public:
	int type;

	std::vector<Scope*> scope_context;
	std::vector<int> node_context;
	int factor_index;
	int obs_index;

	int input_index;

	Input();
	Input(Input& original,
		  Solution* parent_solution);
	Input(std::ifstream& input_file,
		  Solution* parent_solution);

	bool operator==(const Input& rhs);

	void save(std::ofstream& save_file);
};

#endif /* INPUT_H */