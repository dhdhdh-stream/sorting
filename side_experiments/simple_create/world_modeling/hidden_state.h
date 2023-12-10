#ifndef HIDDEN_STATE_H
#define HIDDEN_STATE_H

#include <map>
#include <vector>

#include "run_helper.h"

class Experiment;

class HiddenState {
public:
	double average_val;

	std::map<int, HiddenState*> transitions;

	std::map<int, AbstractExperiment*> experiments;

	HiddenState();
	~HiddenState();

	void activate(HiddenState*& curr_state,
				  std::vector<int>& action_sequence,
				  RunHelper& run_helper);

	void success_reset();
};

#endif /* HIDDEN_STATE_H */