#ifndef MAPPING_HELPERS_H
#define MAPPING_HELPERS_H

#include <vector>

#include "assignment.h"
#include "mapping.h"

void calc_mapping_helper(std::vector<int>& obs,
						 Assignment& assignment,
						 Mapping& mapping);
double calc_conflict_helper(Mapping& mapping);

void init_assignment_helper(std::vector<int>& actions,
							Assignment& assignment);
void modify_assignment_helper(std::vector<int>& actions,
							  Assignment& assignment);

#endif /* MAPPING_HELPERS_H */