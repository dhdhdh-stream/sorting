#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

class Branch {
public:
	bool does_inherit;
	std::vector<std::vector<FoldNetwork*>> score_networks;
	std::vector<bool> is_branch;
	std::vector<BranchPath*> branches;
	std::vector<Fold*> folds;
	// no differing end_indexes, but instead, multiple score_networks per branch

	// TODO: when exploring, branches will not find flats at once, but whenever one does,
	// go upwards, and freeze
	// reference counts flats downwards, and if all fail, move on
	// if success, can keep reference counts and keep going? (because of folds, might be more likely to go to 0) (can also just reset after a certain number of successes)

	// TODO: if update score when fold, but flat when normal branch, then scores can get imbalanced
	// need explore runs vs. update runs?

	// after flat, scores will always be off in front?
	// but updating score networks directly might not have that much meaning, as even if observations newly connected, can't save as state? or maybe good for exploration
	// - but can't capture XOR relations anyways
	//   - so probably not worth, and just scale branches up and down appropriately?
	// but updating score networks in general might cause imbalance? but maybe live with imbalance? and wait for things to adjust?

	// so don't need to keep score networks accurate extremely?
	// - they're there for exploration and then branching
	//   - they don't necessarily stand the test of time
	//     - after the scope is first folded, they are useful, but gradually become less accurate/important?
	//       - but don't know how the new improvement compares to other early branches, so no simple way to update
	//         - to update, have to learn score network
	// - so the mods are just rough helpers? that slowly lose accuracy and purpose, but not necessarily harmful
	//   - mods are just initial helpers for the flats/folds, it's OK to grow out of them

	// divide between flat runs, where force exploration to happen (i.e., don't go down any folds)
	// vs. update runs, where go through as many folds (can be 0) as appropriate, and avoid any EXPLORE_TYPE_NEW
	// - when updating, there's no on-path/off-path distinction

	// maybe if notice observations growing in weight, good time to redo? but XORs can't be captured, so maybe just redo every so often anyways

	// for folding a branch with the front, for the other branches' input, add output networks (like the opposite of the input networks currently)
};

class BranchHistory {
public:
	double best_index;

	FoldNetworkHistory* score_network_history;
	BranchPathHistory* branch_history;
	FoldHistory* fold_history;
};

#endif /* BRANCH_H */