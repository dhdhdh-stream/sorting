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

	// one issue is that sometimes, can perhaps delay state update until fork needed and preserve state?
	// - but this makes reusing the new state harder
	//   - so maybe best and simplest to have each state converge to best as early as possible

	// then an issue might be that will need both original type and new type in the same scope
	// - so might be better to have something different than the additive state network setup

	// for now, don't worry about rewind/just in time stuff
	// - eventually, fetch state only when necessary, whether it is for score or other state
	//   - but for now, pre-fetch everything, even if it's multiple copies of the same state

	// the other option is to take branch possibility into account
	// - but to do this properly is to modify original networks
	//   - like something early matters, but before branch is known, it will then be the average value between the two
	//     - and can use extra state/different types to separate, but would be good to use single state

	// for reused state, can focus on getting the additional state that is needed first
	// - the averaging out affect can happen later?
	//   - make sure that at the time of inner input, the right value is passed in with additional state
	//     - if that additional state can help existing state earlier, then let it fix itself after

	// TODO: have scope initialize variables, and handle recursion properly in experiment?
	// - no, pass by reference is needed
	//   - so everything is pass by reference with initialization done in scope nodes outside
	//     - then easiest to do things the way they have been done

	// any state changes are meant to be used somewhere
	// - if that somewhere is branched off, then possible to reuse state
	//   - otherwise, should create separate

	// maybe it's just that if any changes have to be made, then it's a different type
	// - sure, can average between the two initially, and have it go to the right value on branch, but intuitively, that's like choosing between 2 possibilities (i.e., 2 states) anyways

	// maybe just if 2 states are used together within a scope, add a note
	// - so that on explore, will know to pair variables to pass in
	//   - no, if 2 states are used together *before* the scope, pair them
	//     - intuitively, the scope has expectations for the 2 states before it starts
	//       - the 2 states are not independent coming in

	// for pairing, if from local, look for paired local coming in
	// - if fetch, then fetch together, with dependencies on each other
	//   - if want to mix, then dependency has to be 1-way

	// if there are no pairings, then for fetching, can always use existing, local values?
	// - but what if have pairings within pre inners?
	//   - maybe do just have a giant list of dependencies
	//     - and initialize any that are seen?

	// need pairing only if 2 way
	// - 1 way in either direction doesn't need pairing
	//   - in 1 direction, it doesn't matter, in the other, can use local

	// perhaps if there is mutual dependency, start by representing as a single object
	// - then when further training, use all states from all objects
	//   - if a part of an object is found to have a mutual dependency with something else, split it out as a separate state

	// maybe fetching from memory isn't actually going back through history and doing recalculations
	// - it's just remembered some state already and doing some further calculations for some specific reason
	//   - e.g., if trying to get 1st vs. 2nd vs. etc.
	//     - instead of having separate calculations during some rewind fetch, just save data as state, and do calculations as needed as before

	// so the only things for states for explorations are:
	// - fetching from an outer scope
	//   - don't even fetch from inner
	// - reusing state from earlier new inners in later new inners

	// OK, just going to do something simple:
	// - save the last seen value for every type
	//   - update on scope exit where type is popped
	//     - do so even with new inner
	// - assign the type to the inner input
	//   - so no need to worry about contexts
	// - on experiment, train by adding to the type over full run
	//   - if new network needs to be added, it's a new type
	//     - add new type to outermost scope that's needed as usual
	// - if type has dependencies, indicate that
	//   - then when selecting types, prioritize choosing pairs there too
};

#endif /* OBJECT_DEFINITION_H */