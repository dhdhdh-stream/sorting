#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

class Transformation {
public:
	double scale;
	double weight;

	double pcc;

	double forward(double input);
	double backward(double input);
};

#endif /* TRANSFORMATION_H */