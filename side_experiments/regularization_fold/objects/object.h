#ifndef OBJECT_H
#define OBJECT_H

class Object {
public:

	Type* type;

	std::vector<Type*> dependencies;

	std::map<Type*, Object*> related;

	std::map<int, std::map<int, StateNetwork*>> 
};

#endif /* OBJECT_H */