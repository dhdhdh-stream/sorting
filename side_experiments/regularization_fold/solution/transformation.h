#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

class Transformation {
public:
	double scale;
	double weight;

	double pcc;

	double forward(double input);
	double backward(double input);

	double backprop_backward(double input);	// multiply by scale
	double backprop_forward(double input);	// divide by scale
};

#endif /* TRANSFORMATION_H */