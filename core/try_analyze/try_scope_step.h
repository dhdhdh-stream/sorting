/**
 * - don't worry about matching states
 *   - should tend to match anyways, so hopefully impact will be small
 */

#ifndef TRY_SCOPE_STEP_H
#define TRY_SCOPE_STEP_H

#include <fstream>
#include <utility>
#include <vector>

class TryScopeStep {
public:
	std::vector<std::pair<int, int>> original_nodes;

	TryScopeStep();
	TryScopeStep(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

#endif /* TRY_SCOPE_STEP_H */