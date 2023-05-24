#ifndef INNER_INPUT_H
#define INNER_INPUT_H

const int INNER_INPUT_TYPE_LOCAL = 0;
const int INNER_INPUT_TYPE_FETCH = 1;
const int INNER_INPUT_TYPE_NONE = 2;

class InnerInput {
public:
	int type;

	// for local, don't worry about what happened before
	// - since it's values needed for correctness?
	//   - no, trace it, and build on top of
	//     - may actually need to be a different object
	//     - or don't need to trace it
	//       - just check if there needs to be any modification to the value
	int local_scope_index;
	int scope_object_index;

	int fetch_
	std::vector<>
};

#endif /* INNER_INPUT_H */