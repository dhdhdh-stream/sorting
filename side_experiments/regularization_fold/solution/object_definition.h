#ifndef OBJECT_DEFINITION_H
#define OBJECT_DEFINITION_H

class ObjectDefinition {
public:
	ObjectDefinition* parent;

	int num_total_states;
	int num_overrides;
	std::vector<double> override_indexes;
	int num_new_states;

	// TODO: should this be total or new dependencies?

	// it can always have no dependencies by adding more state to itself

	std::vector<ObjectDefinition*> dependencies;

	// TODO: or don't explicitly track, but just have all inner inputs potentially pull from each other

	// TODO: if 2 objects found to have mutual dependency on each other, can merge?
	// - or it's a child of both

	// the idea with objects is that when reusing an action and looking for input, and there's a difference in what's needed, there are 2 options:
	// (1) pass in a separate set of state and adjust to using that
	// or
	// (2) reuse the same set of state, but use a type to trigger differences in calculation
	// both require the same extra calculation, and both require extra state (type for the latter)
	// - but types are cleaner
	//   - and align intuitively with human intelligence

	// or maybe, humans create objects by visual distance
	// - so objects maybe aren't a thing?

	// because there's almost always mutual dependency
	// - 2 objects come into contact with each other
	//   - they both depend on each other's state
	//     - does that make them the same object?

	// maybe objects are still loosely state that appears and disappears together
	// - their correlation or lack there-of don't really matter

	// should do the same trick with predicted score as with state where each step is trying to get the final value correct?
	// - maybe in recurrent network, don't pass error from a state to itself?
	//   - or maybe lower states don't pass error on to higher states
	//     - higher states push lower states to accommodate to it
	// TODO: experiment on this
	//   - not sure has a big effect
	//     - if connected, difficult to clean up connections even with regularization
	//       - so states may not be clean?

	// probably more effective to just 0 now and then
	// or don't 0, but select earlier state to use instead of just latest
	// - for each state perhaps, with 10% chance, use earlier
	// Works!
	// so can make states best effort approach their values early

	// do recurrent networks need to be fully connected?
	// - or can it be hierarchical?
	//   - no, e.g., 1-state xor repeatedly
};

#endif /* OBJECT_DEFINITION_H */