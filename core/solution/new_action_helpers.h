#ifndef NEW_ACTION_HELPERS_H
#define NEW_ACTION_HELPERS_H

#include "run_helper.h"

class AbstractNode;
class Problem;

void setup_new_action();

void new_action_activate(AbstractNode* experiment_node,
						 AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper);

#endif /* NEW_ACTION_HELPERS_H */