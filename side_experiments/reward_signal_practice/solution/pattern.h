#ifndef PATTERN_H
#define PATTERN_H

#include <fstream>
#include <vector>

class Network;

class Pattern {
public:
	std::vector<int> actions;

	std::vector<int> keypoints;
	std::vector<double> keypoint_averages;
	std::vector<double> keypoint_standard_deviations;

	std::vector<int> inputs;
	Network* network;

	Pattern(std::vector<int> actions,
			std::vector<int> keypoints,
			std::vector<double> keypoint_averages,
			std::vector<double> keypoint_standard_deviations,
			std::vector<int> inputs,
			Network* network);
	Pattern(std::ifstream& input_file);
	~Pattern();

	void save(std::ofstream& output_file);
};

#endif /* PATTERN_H */