/**
 * - for fast update
 *   - e.g., map too slow
 * 
 * TODO: sparse representation + continuous
 */

#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <vector>

class WorldModel {
public:
	double val;

	int min_index;
	int max_index;
	std::vector<WorldModel*> vals;

	WorldModel(std::vector<int>& location,
			   double val);
	WorldModel(WorldModel* original);
	~WorldModel();

	void update(std::vector<int>& location,
				double val);

	void get_val(std::vector<int>& location,
				 bool& is_init,
				 double& val);

	void gather_locations(std::vector<int>& curr_location,
						  std::vector<std::vector<int>>& locations);
};

#endif /* WORLD_MODEL_H */