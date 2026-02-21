#ifndef SIGNAL_INPUT_H
#define SIGNAL_INPUT_H

#include <fstream>
#include <vector>

class Scope;
class Solution;

class SignalInput {
public:
	bool is_pre;
	std::vector<Scope*> scope_context;
	std::vector<int> node_context;
	int obs_index;

	SignalInput();
	SignalInput(SignalInput& original,
				Solution* parent_solution);
	SignalInput(std::ifstream& input_file,
				Solution* parent_solution);

	bool operator==(const SignalInput& rhs) const;
	bool operator!=(const SignalInput& rhs) const;
	bool operator<(const SignalInput& rhs) const;
	bool operator>(const SignalInput& rhs) const;
	bool operator<=(const SignalInput& rhs) const;
	bool operator>=(const SignalInput& rhs) const;

	void print();

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_INPUT_H */