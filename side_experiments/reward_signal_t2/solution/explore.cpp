#include "explore.h"

#include "scope.h"

using namespace std;

Explore::Explore() {
	this->new_scope = NULL;
};

Explore::~Explore() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}
};
