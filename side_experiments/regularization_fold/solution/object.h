#ifndef OBJECT_H
#define OBJECT_H

class Object {
public:
	ObjectDefinition* definition;

	std::vector<double> state_vals;
};

#endif /* OBJECT_H */