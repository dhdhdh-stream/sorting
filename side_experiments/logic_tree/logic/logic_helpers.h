#ifndef LOGIC_HELPERS_H
#define LOGIC_HELPERS_H

#include <vector>

class AbstractLogicNode;
class AbstractProblem;
class LogicTree;
class LogicWrapper;
class SplitNode;

LogicTree* init_helper(AbstractProblem* problem);

double logic_eval_helper(AbstractLogicNode* node,
						 std::vector<double>& obs);
void logic_experiment_helper(AbstractLogicNode* node,
							 std::vector<double>& obs,
							 double target_val,
							 LogicWrapper* logic_wrapper);

void find_parent(AbstractLogicNode* node,
				 LogicTree* logic_tree,
				 SplitNode*& parent,
				 bool& is_branch);
void update_weight_helper(AbstractLogicNode* node,
						  double multiplier);

#endif /* LOGIC_HELPERS_H */