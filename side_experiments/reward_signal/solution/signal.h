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

	Signal();
	Signal(std::ifstream& input_file,
		   Solution* parent_solution);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

	void save(std::ofstream& output_file);
};

#endif /* SIGNAL_H */