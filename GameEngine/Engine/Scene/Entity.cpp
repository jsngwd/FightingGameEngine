#include <iostream>
#include "Entity.h"

Entity::Entity(entt::registry& registry, std::string& name) :
	registry_(registry), name_(name) 
{
	enttId_ = registry_.create();
};

Entity::~Entity()
{
	registry_.remove(enttId_);
	return;
}
