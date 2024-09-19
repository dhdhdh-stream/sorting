#ifndef SEGMENT_HELPERS_H
#define SEGMENT_HELPERS_H

#include <vector>

class Sample;

double calc_distance(Sample* sample,
					 int starting_index,
					 int ending_index);
double calc_distance(Sample* sample,
					 std::vector<int>& starting_indexes,
					 std::vector<int>& ending_indexes);

void find_max_split(Sample* sample,
					std::vector<int>& starting_indexes,
					std::vector<int>& ending_indexes);

#endif /* SEGMENT_HELPERS_H */