#ifndef EXPLORE_UTILITIES_H
#define EXPLORE_UTILITIES_H

#include "solution_node.h"

void set_previous_if_needed(SolutionNode* node,
							SolutionNode* new_previous);

void set_next(SolutionNode* node,
			  SolutionNode* prev_next,
			  SolutionNode* new_next);

#endif /* EXPLORE_UTILITIES_H */