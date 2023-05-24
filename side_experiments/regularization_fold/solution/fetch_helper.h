#ifndef FETCH_HELPER_H
#define FETCH_HELPER_H

#include <vector>

#include "object.h"
#include "scope.h"

void fetch_helper(std::vector<Object>& fetch_obj_vals,
				  std::vector<std::vector<int>>& fetch_dependencies,
				  ScopeHistory* scope_history);

#endif /* FETCH_HELPER_H */