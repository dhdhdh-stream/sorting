#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class WorldState;

class WorldModel {
public:
	std::vector<WorldState*> states;

	std::vector<double> starting_likelihood;

	WorldModel();
	WorldModel(WorldModel* original);
	WorldModel(std::ifstream& input_file);
	~WorldModel();

	void init();

	void clean();

	void random_change();

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

#endif /* WORLD_MODEL_H */