#ifndef SCALE_H
#define SCALE_H

class Scale {
public:
	double weight;
	double weight_update;

	int epoch_iter;
	double average_update_size;

	Scale();
	Scale(double weight);
	~Scale();

	void backprop(double error,
				  double target_max_update);
};

#endif /* SCALE_H */