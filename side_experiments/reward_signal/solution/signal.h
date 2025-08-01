#ifndef SIGNAL_H
#define SIGNAL_H

#include <vector>

#include "input.h"

class Signal {
public:
	int match_factor_index;

	double score_average_val;
	std::vector<Input> score_inputs;
	std::vector<double> score_input_averages;
	std::vector<double> score_input_standard_deviations;
	std::vector<double> score_weights;
};

#endif /* SIGNAL_H */