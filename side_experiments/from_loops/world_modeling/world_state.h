#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include <vector>

class WorldState {
public:
	int id;

	/**
	 * - initialize to overall average_val and average_standard_deviation
	 */
	double average_val;
	double average_standard_deviation;

	std::vector<std::vector<double>> transitions;

	WorldState();

	void forward(double obs,
				 std::vector<double>& curr_likelihoods,
				 int action,
				 std::vector<double>& next_likelihoods);
	void backward(std::vector<double>& curr_likelihoods,
				  int action,
				  double obs,
				  std::vector<double>& next_likelihoods);
};

#endif /* WORLD_STATE_H */