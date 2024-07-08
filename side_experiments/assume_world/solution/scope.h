/**
 * TODO:
 * - when adding back eval, compare ending world against starting world
 */

#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	/**
	 * - tie NewActionExperiment to scope instead of node
	 *   - so that can be tried throughout entire scope
	 */
	NewActionExperiment* new_action_experiment;

};
