#ifndef MINESWEEPER_OPEN_H
#define MINESWEEPER_OPEN_H

#include <vector>

#include "action.h"
#include "scenario.h"

class MinesweeperOpen : public Scenario {
public:
	MinesweeperOpen();
	~MinesweeperOpen();

	void get_attention(std::vector<int>& types,
					   std::vector<Action>& actions,
					   std::vector<std::string>& scopes);
	void get_sequence(std::vector<int>& types,
					  std::vector<Action>& actions,
					  std::vector<std::string>& scopes);

	bool should_perform_sequence();

	std::string get_name();
};

#endif /* MINESWEEPER_OPEN_H */