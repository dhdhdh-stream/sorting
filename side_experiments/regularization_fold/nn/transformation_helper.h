#ifndef TRANSFORMATION_HELPER_H
#define TRANSFORMATION_HELPER_H

class TransformationHelper {
public:
	Transformation transformation;

	double scale_update;
	double weight_update;
	int epoch_iter;
	double average_scale_update_size;
	double average_weight_update_size;

	TransformationHelper();

	void backprop(double val_in,
				  double target);
};

#endif /* TRANSFORMATION_HELPER_H */