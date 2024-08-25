#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include <fstream>
#include <vector>

class WorldState {
public:
	int id;

	/**
	 * - initialize to overall average_val and average_standard_deviation
	 */
	double average_val;
	double average_standard_deviation;

	std::vector<std::vector<std::pair<int,double>>> transitions;

	WorldState();

	void forward(double obs,
				 std::vector<double>& curr_likelihoods,
				 int action,
				 std::vector<double>& next_likelihoods);
	void backward(std::vector<double>& curr_likelihoods,
				  int action,
				  double obs,
				  std::vector<double>& next_likelihoods);

	void transition_step(std::vector<double>& curr_likelihoods,
						 int action,
						 std::vector<double>& next_likelihoods);
	void obs_step(std::vector<double>& curr_likelihoods,
				  double obs);

	void save_for_display(std::ofstream& output_file);
};

#endif /* WORLD_STATE_H */