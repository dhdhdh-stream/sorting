#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

class Transformation {
public:
	double scale;
	double weight;

	double scale_update;
	double weight_update;
	int epoch_iter;
	double average_scale_update_size;
	double average_weight_update_size;

	Transformation();
	Transformation(Transformation* reverse);

	void backprop(double val_in,
				  double target);

	double forward(double val_in);
	double backward(double val_in);

	double backprop_backward(double error_in);	// multiply by scale
	double backprop_forward(double error_in);	// divide by scale
};

#endif /* TRANSFORMATION_H */