#ifndef NEW_ACTION_HELPERS_H
#define NEW_ACTION_HELPERS_H

#include "run_helper.h"

class AbstractNode;
class Problem;
class Solution;

void setup_new_action(Solution* parent_solution);

void new_action_activate(AbstractNode* experiment_node,
						 AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper);

#endif /* NEW_ACTION_HELPERS_H */