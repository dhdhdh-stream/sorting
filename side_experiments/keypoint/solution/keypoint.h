#ifndef KEYPOINT_H
#define KEYPOINT_H

class Keypoint {
public:
	std::vector<Input> inputs;
	Network* network;

	double misguess_standard_deviation;

	double availability;

	int num_hit;
	int num_miss;
	double sum_misguess;



	void measure_update();

};

#endif /* KEYPOINT_H */