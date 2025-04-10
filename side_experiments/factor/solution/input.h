#ifndef INPUT_H
#define INPUT_H

#include <fstream>
#include <vector>

class Scope;
class Solution;

class Input {
public:
	std::vector<Scope*> scope_context;
	std::vector<int> node_context;
	int factor_index;
	int obs_index;

	Input();
	Input(std::ifstream& input_file,
		  Solution* parent_solution);

	bool operator==(const Input& rhs);
	bool operator!=(const Input& rhs);

	void save(std::ofstream& output_file);
};

#endif /* INPUT_H */