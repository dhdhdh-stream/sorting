#ifndef TRANSITION_H
#define TRANSITION_H

#include <fstream>
#include <vector>

class Transition {
public:
	std::vector<int> moves;

	bool likelihood_calculated;
	double success_likelihood;

	Transition();
	Transition(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

#endif /* TRANSITION_H */