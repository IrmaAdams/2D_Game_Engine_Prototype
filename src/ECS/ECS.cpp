#include "ECS.h"
#include "../Logger.h"
#include <algorithm>

int IComponent::nextId = 0;

int Entity::GetId() const {
	return id;
}

void System::AddEntityToSystem(Entity entity) {
	entities.push_back(entity);
}

// iterate values using a Lamda function
void System::RemoveEntityFromSystem(Entity entity) {
	entities.erase(std::remove_if(entities.begin(), entities.end(), [&entity](Entity other) {
		return entity == other; //(using operator overloading)
		}), entities.end());
}

std::vector<Entity> System::GetSystemEntities() const {
	return entities;
}

const Signature& System::GetComponentSignature() const {
	return componentSignature;
}

Entity Registry::CreateEntity() {
	int entityId;

	entityId = numEntities++;

	// Flag new entity to be created before the next frame
	Entity entity(entityId);
	entitiesToBeAdded.insert(entity);

	Logger::Log("Entity created with id = " + std::to_string(entityId));

	return entity;

}

void Registry::Update() {
	// Add the entities that are waiting to be created to the active systems
	// Remove the entities that are waiting to be killed from the active systems


}