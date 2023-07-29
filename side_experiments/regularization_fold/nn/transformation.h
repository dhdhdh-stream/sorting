#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <fstream>

class Transformation {
public:
	double scale;
	double weight;

	Transformation();
	Transformation(Transformation& reverse);
	Transformation(std::ifstream& save_file);

	double forward(double val_in);
	double backward(double val_in);

	double backprop_backward(double error_in);	// multiply by scale
	double backprop_forward(double error_in);	// divide by scale

	void save(std::ofstream& save_file);
};

#endif /* TRANSFORMATION_H */