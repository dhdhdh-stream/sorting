#ifndef TRY_ANALYZE_HELPERS_H
#define TRY_ANALYZE_HELPERS_H

void try_distance(TryInstance* original,
				  TryInstance* potential,
				  double& distance,
				  std::vector<std::pair<int, std::pair<int, int>>>& diffs);

#endif /* TRY_ANALYZE_HELPERS */