#ifndef MEASURE_INFORMATION_H
#define MEASURE_INFORMATION_H

#include <vector>

#include "action.h"
#include "solution_node.h"

double measure_information(std::vector<int> pre_sequence,
						   SolutionNode* root_node,
						   std::vector<SolutionNode*> nodes,
						   std::vector<Action> candidate);

#endif /* MEASURE_INFORMATION_H */