#ifndef CONDITION_WAS_START_H
#define CONDITION_WAS_START_H

#include "abstract_condition.h"

class ConditionWasStart : public AbstractCondition {
public:
	int start_index;

	ConditionWasStart(int start_index);
	ConditionWasStart(ConditionWasStart* original);
	ConditionWasStart(std::ifstream& input_file);

	bool is_hit(std::vector<std::vector<double>>& obs_history,
				std::vector<int>& move_history);

	void print();

	void save(std::ofstream& output_file);
};

#endif /* CONDITION_WAS_START_H */