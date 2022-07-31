#ifndef EXPLORE_H
#define EXPLORE_H

#include <random>
#include <vector>

#include "action.h"
#include "solution_node.h"

std::vector<Action> explore(std::vector<int> pre_sequence,
							SolutionNode* root_node,
							std::vector<SolutionNode*> nodes,
							std::default_random_engine generator);

#endif /* EXPLORE_H */