#ifndef ABSTRACT_CONDITION_H
#define ABSTRACT_CONDITION_H

#include <fstream>
#include <vector>

const int CONDITION_TYPE_OBS_GREATER_THAN = 0;
const int CONDITION_TYPE_OBS_LESS_THAN = 1;
const int CONDITION_TYPE_OBS_WITHIN = 2;
const int CONDITION_TYPE_ACTION_EQUALS = 3;
const int CONDITION_TYPE_WAS_START = 4;

class AbstractCondition {
public:
	int type;

	virtual ~AbstractCondition() {};

	virtual bool is_hit(std::vector<std::vector<double>>& obs_history,
						std::vector<int>& move_history) = 0;

	virtual void save(std::ofstream& output_file) = 0;
};

#endif /* ABSTRACT_CONDITION_H */