// - transformations lead to the same solution...
//   - until something additional is found, which means result is new/better/unique

#ifndef ABSTRACT_TRANSFORMATION_H
#define ABSTRACT_TRANSFORMATION_H

const int TRANSFORMATION_TYPE_ADD = 0;
const int TRANSFORMATION_TYPE_MOVE = 1;
// - or rearrange?
//   - maybe have various binary trees of rearrangements downwards?
//     - once split, cannot cross over?
//     - within subtree, recurse

// - within a subtree, nodes can be rearranged in any order
//   - (if can't be rearranged in any order, group into its own subtree)
//     - but how to do so if graph?
// - or would be a sequence, with parts that have to be in order, and nodes where within, everything is free form

// - for graph, do preprocessing
//   - group everything within branch and furthest exit
//     - try to move as much out of grouping as possible

// - then, a transformation is a rearrangement followed by a bunch of additions

// - currently, solution very bad at gathering information
//   - a lot of it is within scopes
//   - so bad at early exits

// - don't save scope histories, but can pass requests for input in?

class AbstractTransformation {
public:
	int type;

	std::vector<AbstractTransformation*> follow_ups;

};

#endif /* ABSTRACT_TRANSFORMATION_H */