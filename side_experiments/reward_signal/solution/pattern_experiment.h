#ifndef PATTERN_EXPERIMENT_H
#define PATTERN_EXPERIMENT_H

#include <map>
#include <vector>

#include "input.h"

const double PATTERN_MIN_MATCH_PERCENT = 0.15;
const double PATTERN_MAX_MATCH_PERCENT = 0.20;

class Pattern;
class ScopeHistory;

class PatternExperiment {
public:
	Scope* scope_context;

	std::vector<ScopeHistory*> existing_scope_histories;

	std::vector<ScopeHistory*> explore_scope_histories;
	std::vector<double> explore_target_vals;

	PatternExperiment(Scope* scope_context);
	~PatternExperiment();

	void eval();

private:
	bool train_keypoints_helper(Pattern* potential_pattern);
	double train_and_eval_predict_helper(Pattern* potential_pattern);
};

#endif /* PATTERN_EXPERIMENT_H */