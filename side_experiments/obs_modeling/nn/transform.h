#ifndef TRANSFORM_H
#define TRANSFORM_H

class Transform {
public:
	double scale;
	double offset;

	Transform(double scale,
			  double offset);

	double activate(double input);
};

#endif /* TRANSFORM_H */