#include "return_node.h"

#include "minesweeper.h"

using namespace std;

void ReturnNode::explore_activate(Problem* problem,
								  vector<ContextLayer>& context) {
	if (this->previous_location != NULL) {
		map<AbstractNode*, pair<int,int>>::iterator it
			= context.back().location_history.find(this->previous_location);
		if (it != context.back().location_history.end()) {
			Minesweeper* minesweeper = (Minesweeper*)problem;
			minesweeper->current_x = it->second.first;
			minesweeper->current_y = it->second.second;
		}
	}
}
