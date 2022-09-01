#include "fold_helper.h"

using namespace std;

FoldHelper::FoldHelper(int layer) {
	this->layer = layer;
	this->input_indexes = new NDVector(layer);
}

FoldHelper::~FoldHelper() {
	delete this->input_indexes;
}

void FoldHelper::process(double obs,
						 double* network_input,
						 vector<int>& loop_scope_counts) {
	int index;
	this->input_indexes->get_value(loop_scope_counts, 0, index);
	network_input[index] = obs;
}

NDVector::NDVector(int height) {
	this->height = height;
	if (this->height > 1) {
		for (int i = 0; i < 5; i++) {
			this->inner.push_back(new NDVector(height-1));
		}
	}
}

NDVector::~NDVector() {
	for (int i = 0; i < 5; i++) {
		delete this->inner[i];
	}
}

void NDVector::set_value(vector<int>& index,
						 int curr,
						 int value) {
	if (this->height == 1) {
		this->value = value;
		return;
	}

	this->inner[index[curr]]->set_value(index, curr+1, value);
}

void NDVector::get_value(vector<int>& index,
						 int curr,
						 int& value) {
	if (this->height == 1) {
		value = this->value;
		return;
	}

	this->inner[index[curr]]->get_value(index, curr+1, value);
}
