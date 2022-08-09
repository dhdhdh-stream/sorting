#ifndef LOOP_DICTIONARY_H
#define LOOP_DICTIONARY_H

#include <mutex>

class LoopDictionary {
public:
	int loop_counter;

	std::vector<Loop*> established;

	std::mutex mtx;
};

#endif /* LOOP_DICTIONARY_H */