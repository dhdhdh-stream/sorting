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
};

#endif /* FETCH_H */