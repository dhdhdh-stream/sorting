#ifndef HIDDEN_STATE_H
#define HIDDEN_STATE_H

#include <fstream>
#include <map>
#include <vector>

#include "run_helper.h"

class Experiment;

class HiddenState {
public:
	int id;

	double average_val;

	std::map<int, HiddenState*> transitions;

	std::map<int, AbstractExperiment*> experiments;

	HiddenState();
	~HiddenState();

	void activate(HiddenState*& curr_state,
				  std::vector<int>& action_sequence,
				  RunHelper& run_helper);

	void success_reset();

	void save_for_display(std::ofstream& output_file);
};

#endif /* HIDDEN_STATE_H */