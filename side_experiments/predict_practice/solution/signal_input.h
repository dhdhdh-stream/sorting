#ifndef SIGNAL_INPUT_H
#define SIGNAL_INPUT_H

#include <fstream>
#include <vector>

class Scope;
class Solution;
class WorldModel;

class SignalInput {
public:
	int x_coord;
	int y_coord;
	bool check_is_on;

	SignalInput();
	SignalInput(const SignalInput& original);
	SignalInput(std::ifstream& input_file);

	bool operator==(const SignalInput& rhs) const;
	bool operator!=(const SignalInput& rhs) const;
	bool operator<(const SignalInput& rhs) const;
	bool operator>(const SignalInput& rhs) const;
	bool operator<=(const SignalInput& rhs) const;
	bool operator>=(const SignalInput& rhs) const;

	void activate(WorldModel* world_model,
				  double& val,
				  bool& is_on);

	void print();

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_INPUT_H */