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
	entity.registry = this;
	entitiesToBeAdded.insert(entity);


	// make sure the entityComponentsignatures vector can accomodate the new entity
	if (entityId >= entityComponentSignatures.size()) {
		entityComponentSignatures.resize(entityId + 1);
	}

	Logger::Log("Entity created with id = " + std::to_string(entityId));

	return entity;

}

void Registry::AddEntityToSystems(Entity entity) {
	const auto entityId = entity.GetId();

	const auto& entityComponentSignature = entityComponentSignatures[entityId];

	// Loop all the systems
	for (auto& system : systems) {
		const auto& systemComponentSignature = system.second->GetComponentSignature();  // system.second = system is an iterator pointer, acces by system.second

		// compare 2 bitsets
		bool isInterested = (entityComponentSignature & systemComponentSignature) == systemComponentSignature;

		if (isInterested){
			system.second->AddEntityToSystem(entity);
		}
	}
}

void Registry::Update() {
	// Add the entities that are waiting to be created to the active systems
	for (auto entity : entitiesToBeAdded) {
		AddEntityToSystems(entity);
	}
	entitiesToBeAdded.clear();
	
	// Remove the entities that are waiting to be killed from the active systems


}