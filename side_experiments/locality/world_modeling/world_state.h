#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include <fstream>
#include <vector>

class WorldModel;

class WorldState {
public:
	WorldModel* parent;
	int id;

	double average_val;

	std::vector<std::vector<std::pair<WorldState*,double>>> transitions;

	WorldState();

	void forward(double obs,
				 std::vector<double>& curr_likelihoods,
				 int action,
				 std::vector<double>& next_likelihoods,
				 double& next_unknown_likelihood);
	void backward(std::vector<double>& curr_likelihoods,
				  double curr_unknown_likelihood,
				  int action,
				  double obs,
				  std::vector<double>& next_likelihoods);
	void update(std::vector<std::vector<double>>& sum_transitions);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);

	void copy_from(WorldState* original);

	void save_for_display(std::ofstream& output_file);
};

#endif /* WORLD_STATE_H */