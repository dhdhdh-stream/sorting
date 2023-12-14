#include "transform.h"

using namespace std;

Transform::Transform(double scale,
					 double offset) {
	this->scale = scale;
	this->offset = offset;
}

double Transform::activate(double input) {
	return this->scale * input + this->offset;
}
