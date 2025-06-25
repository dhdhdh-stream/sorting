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

	bool operator==(const Input& rhs) const;
	bool operator!=(const Input& rhs) const;
	bool operator<(const Input& rhs) const;
	bool operator>(const Input& rhs) const;
	bool operator<=(const Input& rhs) const;
	bool operator>=(const Input& rhs) const;

	void print();

	void save(std::ofstream& output_file);
};

class InputData {
public:
	double hit_percent;
	double average;
	double standard_deviation;
};

#endif /* INPUT_H */