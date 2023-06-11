#ifndef FETCH_H
#define FETCH_H

// const int FETCH_BRANCH_CHOICES_ORIGINAL = -1;
const int FETCH_BRANCH_CHOICES_BRANCH = 0;
const int FETCH_BRANCH_CHOICES_ANY = 1;

class Fetch {
public:
	Scope* outer_scope;

	std::vector<std::map<int, int>> fetch_branch_choices;
	/**
	 * - if node not in branch_choices, choose original
	 *   - so new nodes will go to original
	 * - if new branch end seen, add branch node as any
	 */

	// TODO: for branch/branch end, need to track whether coming from new or original?
	// - actually, branches are not hierarchical, so not guaranteed to have corresponding branch/branch ends

	// TODO: actually, don't even need branch ends anymore

	// Actually, for a fetch, just register if a branch node has been seen previously
	// - by seen previously, can come from normal activate
	//   - so though fetch won't trigger anything new, normal activate still will

	// maybe just simply run and see if hit
	// - later, maybe some explore/experiment will figure out how to early exit if missed

	// also, basing fetches around scopes might be inflexible?
	// - vs. doing something based around sequences?

	// TODO: track for non-loop scopes
	// - and for loop scopes, if finished without hitting, then can early exit

	// TODO: maybe don't do fetch, and just rely on sequences to initialize things correctly
	// - that means should keep track of where nodes come from, and reuse state networks
	//   - yeah, and also rely heavier on last seen
	//     - for last seen, maybe use initially, but then scale down

	std::vector<int> ending_node_ids;

	// TODO: update scope score networks on fetch
	// - it's as if reusing scope + new branch

	// but don't explore?
	// - or can explore as long as don't jump out?
	//   - then need to add extra bookkeeping

	// intuitively, splitting into action scopes and fetch scopes makes sense
	// - so they can be used and treated differently
	//   - or just keep as is
	//     - allow explore with extra bookkeeping
	// - or actually, can add a flag at end node, so fetch can just check that instead of keeping giant map

	// TODO: fetchs should end at points with high information

	// actually, with loops, difficult to decide where to stop?
	// - maybe, can't be within a loop
	//   - don't need loop? and can instead, hope to stop after loop?
	// - issue with loops is the random NONE-ing to keep iters diverse during exploring
	//   - fetch is like exploring, but with the assumption that things already make sense
	//     - so doesn't match
	// - but not having loops anywhere along the context path is restricting

	// OK, maybe if state from loop, then just pass none or wait until end

	// then, if there are loops outside, just use the first instance in which state is initialized
	// - so fetches can miss

	// branching in explores should be OK
	// - if new branch can lead to a miss, will hopefully learn to only branch when appropriate
};

#endif /* FETCH_H */