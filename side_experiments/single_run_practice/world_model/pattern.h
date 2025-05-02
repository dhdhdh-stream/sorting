/**
 * - combination of states that have been measured to be unique
 *   - otherwise, could be duplicates
 */

#ifndef PATTERN_H
#define PATTERN_H

#include <fstream>
#include <vector>

class FixedPoint;

class Pattern {
public:
	std::vector<FixedPoint*> fixed_points;

	Pattern();
	Pattern(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

#endif /* PATTERN_H */