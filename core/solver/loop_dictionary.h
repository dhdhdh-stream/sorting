#ifndef LOOP_DICTIONARY_H
#define LOOP_DICTIONARY_H

#include <mutex>
#include <vector>

#include "loop.h"

class LoopDictionary {
public:
	int loop_counter;

	std::vector<Loop*> established;

	std::mutex counter_mtx;
	std::mutex established_mtx;

	LoopDictionary();
	LoopDictionary(std::ifstream& save_file);
	~LoopDictionary();

	void save(std::ofstream& save_file);
};

#endif /* LOOP_DICTIONARY_H */