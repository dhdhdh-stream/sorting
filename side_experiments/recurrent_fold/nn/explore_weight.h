/**
 * identical to Scale, but can't go below 0.0
 */

#ifndef EXPLORE_WEIGHT_H
#define EXPLORE_WEIGHT_H

class ExploreWeight {
public:
	double weight;
	double weight_update;

	int epoch_iter;
	double average_update_size;

	ExploreWeight();
	ExploreWeight(double weight);
	~ExploreWeight();

	void backprop(double error,
				  double target_max_update);
};

#endif /* EXPLORE_WEIGHT_H */