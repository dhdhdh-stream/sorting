#include "transformation.h"

#include <iostream>

using namespace std;

Transformation::Transformation() {
	this->scale = 0.0;
	this->weight = 0.0;
}

Transformation::Transformation(ifstream& input_file) {
	string scale_line;
	getline(input_file, scale_line);
	this->scale = stod(scale_line);

	string weight_line;
	getline(input_file, weight_line);
	this->weight = stod(weight_line);
}

double Transformation::forward(double val_in) {
	return this->scale*val_in + this->weight;
}

double Transformation::backward(double val_in) {
	return (val_in-this->weight)/this->scale;
}

double Transformation::backprop_backward(double error_in) {
	return this->scale*error_in;
}

double Transformation::backprop_forward(double error_in) {
	return error_in/this->scale;
}

void Transformation::save(ofstream& output_file) {
	output_file << this->scale << endl;
	output_file << this->weight << endl;
}
