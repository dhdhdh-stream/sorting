#ifndef FETCH_H
#define FETCH_H



class FetchInput {
public:

	// issue with local dependency is what if it has its dependencies that depend on changes
	// cannot have local dependencies then?
	// just have 0s?

	// can take in state from outside though

	// anywhere where type could be applied, replace
	// if all the way from the outside, then clear first?

	// maybe object is a grouping of states which have recurrent dependencies on each other
	// - and then they also have some non-recurrent dependencies

	// then instead of checking a state, check if entire object is there

	// then there may be sub-objects, which have some sub-dependencies, and may have some new dependencies

	// instead of states having dependencies on states, objects can have dependencies on objects

	// when determining what state networks to use, use one that matches object along with all its parents

	// for dependencies, can then use existing

	// even if object covers multiple scopes, and in some of those scopes, states don't matter, still include in same object
	// - when fetching, just leave 0 at start, and potentially never touch
	//   - then is there even a point of removing from scope?
	//     - or just treat inputs as objects instead of state from the start

	// can also treat internal and external state differently?
	// - no, everything connected should be treated as one object
	//   - can divide deeper later

	// after training, should mark which scopes don't need object

	std::vector<int> dependency_types;
};

class Fetch {
public:


};

#endif /* FETCH_H */