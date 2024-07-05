#ifndef OBJECT_H
#define OBJECT_H

const int DIMENSION_TYPE_LINEAR = 0;
/**
 * - for input, simply go back to 0 when circled back
 *   - state responsible for knowing when circled back
 * 
 * - for recurrent input, cross 0 boundary
 */
const int DIMENSION_TYPE_LOOP = 1;
const int DIMENSION_TYPE_GRAPH = 2;

class Object {
public:
	std::vector<int> dimension_types;


};

#endif /* OBJECT_H */