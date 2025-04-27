#ifndef CONDITION_ACTION_EQUALS_H
#define CONDITION_ACTION_EQUALS_H

#include "abstract_condition.h"

class ConditionActionEquals : public AbstractCondition {
public:
	int action_index;
	int move;

	ConditionActionEquals(int action_index,
						  int move);
	ConditionActionEquals(ConditionActionEquals* original);
	ConditionActionEquals(std::ifstream& input_file);

	bool is_hit(std::vector<std::vector<double>>& obs_history,
				std::vector<int>& move_history);

	void print();

	void save(std::ofstream& output_file);
};

#endif /* CONDITION_ACTION_EQUALS */