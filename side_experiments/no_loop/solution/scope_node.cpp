#include "scope_node.h"

using namespace std;





ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;

	this->is_halfway = false;
}
