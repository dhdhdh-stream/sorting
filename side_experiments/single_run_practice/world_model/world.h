// - if failure/noise purely random, then just noise
//   - but if there is pattern, create "object"/"event" to capture pattern

#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <string>
#include <vector>

class FixedPoint;
class Pattern;

class World {
public:
	double val_average;
	double val_variance;

	int fixed_point_counter;
	std::map<int, FixedPoint*> fixed_points;

	std::vector<Pattern*> patterns;

	World();
	~World();

	void init();
	void load(std::string path,
			  std::string name);

	void save(std::string path,
			  std::string name);
};

#endif /* WORLD_H */