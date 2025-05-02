/**
 * - initially, transition probabilities given by looping back to self
 *   - then, can use this loop back probability to calculate other probabilities
 *     - e.g., probability of reaching and looping / probability of looping
 */

#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <fstream>
#include <map>

#include "transition.h"

class FixedPoint {
public:
	int id;

	double val_average;

	std::map<FixedPoint*, Transition> transitions;

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
};

#endif /* FIXED_POINT_H */