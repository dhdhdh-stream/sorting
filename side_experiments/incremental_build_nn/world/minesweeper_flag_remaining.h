/**
 * TODO:
 * - actually, still useful to learn state separate from actions
 *   - a lot of decisions need to be learned indirectly
 *     - e.g., learn number of remaining, learn number of open, then compare
 */

// learn state and state transformations
// compare states, transformation of states, etc., to make decisions

// regardless of new sequence, can always try to learn indirect
// - new indirect may apply only to that sequence -- or it may apply generally

// but, e.g., flag remaining requires paying attention to the surrounding eight squares and comparing it to the center
// - why would it be worth it to try to pay attention to something like this without a payoff?

// maybe just improve learning
// - focus on mistakes

#ifndef MINESWEEPER_FLAG_REMAINING_H
#define MINESWEEPER_FLAG_REMAINING_H

#include <vector>

#include "action.h"
#include "scenario.h"

class MinesweeperFlagRemaining : public Scenario {
public:
	MinesweeperFlagRemaining();
	~MinesweeperFlagRemaining();

	void get_attention(std::vector<int>& types,
					   std::vector<Action>& actions,
					   std::vector<std::string>& scopes);
	void get_sequence(std::vector<int>& types,
					  std::vector<Action>& actions,
					  std::vector<std::string>& scopes);

	bool should_perform_sequence();

	std::string get_name();
};

#endif /* MINESWEEPER_FLAG_REMAINING_H */