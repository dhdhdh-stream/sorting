/**
 * TODO:
 * - actually, still useful to learn state separate from actions
 *   - a lot of decisions need to be learned indirectly
 *     - e.g., learn number of remaining, learn number of open, then compare
 */

// learn state and state transformations
// compare states, transformation of states, etc., to make decisions

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