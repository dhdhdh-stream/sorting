#ifndef KEYPOINT_H
#define KEYPOINT_H

#include <vector>

#include "input.h"
#include "run_helper.h"

class Network;
class ScopeHistory;

class Keypoint {
public:
	std::vector<Input> inputs;
	Network* network;

	double misguess_standard_deviation;

	double availability;

	int num_hit;
	int num_miss;
	double sum_misguess;

	Keypoint();
	Keypoint(Keypoint* original,
			 Solution* parent_solution);
	~Keypoint();

	void experiment_activate(double target_val,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void measure_activate(double target_val,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history);

	bool should_clean_inputs(Scope* scope,
							 int node_id);
	bool should_clean_inputs(Scope* scope);

	void clean();
	void measure_update();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
};

#endif /* KEYPOINT_H */