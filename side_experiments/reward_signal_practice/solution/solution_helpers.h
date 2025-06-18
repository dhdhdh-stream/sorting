#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <vector>

class Network;

void measure_keypoints(std::vector<int>& full_sequence,
					   int pattern_start_index,
					   std::vector<int>& keypoints,
					   std::vector<double>& keypoint_averages,
					   std::vector<double>& keypoint_standard_deviations);

void train_pattern_network(std::vector<int>& actions,
						   std::vector<int>& keypoints,
						   std::vector<double>& keypoint_averages,
						   std::vector<double>& keypoint_standard_deviations,
						   std::vector<int>& inputs,
						   Network*& network,
						   double& average_misguess,
						   double& misguess_standard_deviation);

#endif /* SOLUTION_HELPERS_H */