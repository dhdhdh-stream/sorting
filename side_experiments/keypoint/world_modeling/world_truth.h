/**
 * - world details:
 *   - 3 x 3 to 5 x 5 grid (with additional border)
 *   - 1 x 1 object potentially with velocity moving around
 *   - chance of getting stuck
 *   - obs from 2 iters in the past
 *   - random starting location
 */

#ifndef WORLD_TRUTH_H
#define WORLD_TRUTH_H

#include <fstream>
#include <vector>

class WorldTruth {
public:
	int world_size;
	int curr_x;
	int curr_y;

	int obj_x;
	int obj_y;
	int obj_x_vel;
	int obj_y_vel;

	std::vector<int> action_queue;

	WorldTruth();

	void init();

	double get_obs();
	void move(int action);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
};

#endif /* WORLD_TRUTH_H */