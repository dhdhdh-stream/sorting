#ifndef TYPE_H
#define TYPE_H

#include <vector>

class Type {
public:
	// this should be a hierarchy?

	// so scope inputs have a type

	// there needs to be actions that are type specific
	// if of a certain type, do something, otherwise, do something else
	// - train networks on top of original, and try to trim

	// when extending, if have to add on top of original state networks, then new variant
	// - otherwise, same object

	// then fetches need to run code, which is fine
	// - have full dependencies + start with zero state

	std::vector<Object*> objects;
};

#endif /* TYPE_H */