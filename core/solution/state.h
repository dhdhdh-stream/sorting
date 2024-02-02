// there has to be a certain structure that humans look for
// - to be able to create clean signals from a mix

// perhaps additive but not dependent?
// - means stable against different paths
// - still reduces number of state to keep track of

// - then have only linear combinations of state
//   - but can combine states to form new state
//     - but has to be strictly hierarchical, i.e, tree
//       - to prevent circular dependencies

// how to extend state?
// - only way is if matches original decision making?
//   - along original path

// if new path anywhere, then problem changes, so new relation may be needed
// - so can always "find" an extension even when not ideal

// e.g., if original path uses object A, and now take different path
// - expect there to be object A
//   - so find and use object B, as it somewhat matches the need
//     - but object A is not object B

// maybe goes back again to world modeling
// - and identity is somehow key
//   - for humans, that is so tied to spatial awareness though
//     - is that really needed for solution building?
//       - no way 3D is key

// perhaps just incrementally build neural networks
// - train to allow 0ing
//   - fix existing, and add to
// - can turn off sections of the network if not relevant

// then have lots of backtracking

// and essentially have no "inputs" and see how it goes?

// the thing with creating state is that enables linear regression
// - which is much faster
// - maybe go giant linear regression into nn?

#ifndef STATE_H
#define STATE_H

#include <fstream>
#include <set>
#include <vector>

class AbstractNode;
class StateNetwork;
class Scope;

class State {
public:
	int id;

	std::vector<StateNetwork*> networks;

	State();
	State(std::ifstream& input_file,
		  int id);
	~State();

	void save(std::ofstream& output_file);
};

#endif /* STATE_H */