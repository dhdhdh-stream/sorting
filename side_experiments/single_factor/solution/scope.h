/**
 * - scopes are just a bit of abstraction to try to promote the reuse of ideas
 *   - scopes roughly capture when certain state is relevant, but is extremely imprecise
 *     - the actual corresponding actions may start sooner or later, and may end sooner or later
 *   - in addition, may need to constantly break up scopes to use elsewhere
 *   - but as long as scopes, inner scopes, etc., are being created and reused, should be good enough to make progress
 * 
 * - specifically, here, the states created are too loose
 *   - i.e., they likely include more actions than is relevant for the state
 *     - so from the outside, there's more that's abstracted, but from the inside, will miss possible sub-scopes
 *       - though sub-scopes will likely be created on branch
 *         - so again, probably good enough
 */

#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	int id;

	int num_states;
	/**
	 * - when new state added, can be added twice
	 *   - once as local
	 *   - once as input
	 */
	std::vector<bool> state_initialized_locally;

	/**
	 * - alternate way of storing initialized locally information for scope nodes
	 */
	std::vector<int> initialized_locally_indexes;
	std::vector<Scale*> ending_score_scales;

	std::vector<std::vector<int>> classes_used;


};

#endif /* SCOPE_H */