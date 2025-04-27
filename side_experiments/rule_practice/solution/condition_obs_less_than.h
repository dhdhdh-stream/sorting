#ifndef CONDITION_OBS_LESS_THAN_H
#define CONDITION_OBS_LESS_THAN_H

#include "abstract_condition.h"

class ConditionObsLessThan : public AbstractCondition {
public:
	int obs_index;
	double min_val;

	ConditionObsLessThan(int obs_index,
						 double max_val);
	ConditionObsLessThan(ConditionObsLessThan* original);
	ConditionObsLessThan(std::ifstream& input_file);

	bool is_hit(std::vector<std::vector<double>>& obs_history,
				std::vector<int>& move_history);

	void save(std::ofstream& output_file);
};

#endif /* CONDITION_OBS_LESS_THAN_H */