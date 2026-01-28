#ifndef LOGIC_HELPERS_H
#define LOGIC_HELPERS_H

#include <vector>

class AbstractLogicNode;
class AbstractProblem;
class LogicTree;
class LogicWrapper;
class SplitNode;

bool is_match_helper(std::vector<double> obs,
					 int obs_index,
					 int rel_obs_index,
					 int split_type,
					 double split_target,
					 double split_range);

LogicTree* init_helper(AbstractProblem* problem);

double logic_eval_helper(AbstractLogicNode* node,
						 std::vector<double>& obs);
void logic_experiment_helper(std::vector<double>& obs,
							 double target_val,
							 LogicWrapper* logic_wrapper,
							 AbstractProblem* problem);
double view_logic_eval_helper(AbstractLogicNode* node,
							  std::vector<double>& obs);

void update(AbstractLogicNode* node,
			LogicWrapper* wrapper);

double measure_helper(AbstractProblem* problem,
					  LogicWrapper* logic_wrapper);

#endif /* LOGIC_HELPERS_H */