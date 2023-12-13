#ifndef EXPERIMENT_H
#define EXPERIMENT_H

class Experiment : public AbstractExperiment {
public:
	std::vector<WorldState*> experiment_states;
	/**
	 * - starting is experiment_states[0]
	 */

	int state;
	int state_iter;

	double existing_average_misguess;
	double existing_misguess_variance;

	std::map<WorldState*, int> experiment_state_reverse_mapping;
	std::vector<std::vector<double>> ending_vals;
	std::vector<std::vector<std::vector<double>>> ending_state_vals;
	std::vector<double> ending_val_averages;
	std::vector<std::vector<double>> ending_state_val_impacts;

	double new_misguess;

	std::vector<double> misguess_histories;


};

#endif /* EXPERIMENT_H */