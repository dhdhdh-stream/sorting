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

void simple_assign(Mapping& mapping,
				   std::vector<std::vector<int>>& instance);

void simple_assign(std::vector<int>& obs,
				   std::vector<int>& actions,
				   std::vector<std::vector<int>>& mapping);

bool check_valid(std::vector<int>& obs,
				 std::vector<int>& actions,
				 std::vector<std::vector<int>>& mapping);

bool check_valid(std::vector<int>& obs,
				 std::vector<int>& actions,
				 std::vector<std::vector<int>>& map_vals,
				 std::vector<std::vector<bool>>& map_assigned);

#endif /* MAPPING_HELPERS_H */