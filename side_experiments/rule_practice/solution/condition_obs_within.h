#ifndef CONDITION_OBS_WITHIN_H
#define CONDITION_OBS_WITHIN_H

#include "abstract_condition.h"

class ConditionObsWithin : public AbstractCondition {
public:
	int obs_index;
	double min_val;
	double max_val;

	ConditionObsWithin(int obs_index,
					   double min_val,
					   double max_val);
	ConditionObsWithin(ConditionObsWithin* original);
	ConditionObsWithin(std::ifstream& input_file);

	bool is_hit(std::vector<std::vector<double>>& obs_history,
				std::vector<int>& move_history);

	void print();

	void save(std::ofstream& output_file);
};

#endif /* CONDITION_OBS_WITHIN */