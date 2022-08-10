#include "loop_dictionary.h"

using namespace std;

LoopDictionary::LoopDictionary() {
	this->loop_counter = 0;
}

LoopDictionary::LoopDictionary(ifstream& save_file) {
	string num_established_line;
	getline(save_file, num_established_line);
	int num_established = stoi(num_established_line);
	for (int l_index = 0; l_index < num_established; l_index++) {
		Loop* loop = new Loop(save_file);
		this->established.push_back(loop);
	}

	string loop_counter_line;
	getline(save_file, loop_counter_line);
	this->loop_counter = stoi(loop_counter_line);
}

LoopDictionary::~LoopDictionary() {
	for (int l_index = 0; l_index < (int)this->established.size(); l_index++) {
		delete this->established[l_index];
	}
}

void LoopDictionary::save(ofstream& save_file) {
	this->established_mtx.lock();
	int num_established = (int)this->established.size();
	this->established_mtx.unlock();

	save_file << num_established << endl;
	for (int l_index = 0; l_index < num_established; l_index++) {
		this->established[l_index]->save(save_file);
	}
	save_file << this->loop_counter << endl;
}
