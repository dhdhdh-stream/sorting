#ifndef TRY_ANALYZE_HELPERS_H
#define TRY_ANALYZE_HELPERS_H

#include <utility>
#include <vector>

class TryInstance;
class TryScopeStep;

void try_distance(TryInstance* original,
				  TryInstance* potential,
				  double& distance,
				  std::vector<std::pair<int, std::pair<int,int>>>& diffs);

void try_scope_step_diff(TryScopeStep* original,
						 TryScopeStep* potential,
						 std::vector<std::pair<int, int>>& additions,
						 std::vector<std::pair<int, int>>& removals);

#endif /* TRY_ANALYZE_HELPERS */