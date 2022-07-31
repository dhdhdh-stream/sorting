#ifndef SETUP_DECISION_MAKING_H
#define SETUP_DECISION_MAKING_H

#include <vector>

#include "action.h"
#include "solution_node.h"

// assuming no loops
void setup_decision_making(SolutionNode* target_node,
						   std::vector<int> pre_sequence,
						   SolutionNode* root_node,
						   std::vector<SolutionNode*> nodes);

#endif /* SETUP_DECISION_MAKING_H */