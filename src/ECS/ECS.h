#ifndef ECS_H
#define ECS_H

#include "../Logger.h"
#include <vector>
#include <bitset>
#include <set>
#include <unordered_map>
#include <typeindex>
#include <memory>

const unsigned int MAX_COMPONENTS = 32;
//*************************************************************************************
// SIGNATURE
// Use a bitset (1s and 0s) to keep track of which components an entity has,
// and also helps keep track of which entities a system is interested in.
//*************************************************************************************

typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent {
protected:
	static int nextId;
};

// used to assign a unique id to a component type
// Component Class Template:
template <typename T>
class Component: public IComponent {
	// returns the unique id of Component<T>
public:
	static int GetId() {
		static auto id = nextId++;
		return id;
	}
};

// Wrapper class around an entity id with forward declaration:
class Entity {
private:
	int id;

public:
	Entity(int id): id(id) {};		// initialize member variables directly
	Entity(const Entity& entity) = default;
	int GetId() const;

	// Operator overloading for entities:
	Entity& operator = (const Entity& other) = default;
	bool operator == (const Entity& other) const { return id == other.id; }
	bool operator != (const Entity& other) const { return id != other.id; }
	bool operator > (const Entity& other) const { return id > other.id; }
	bool operator < (const Entity& other) const { return id < other.id; }

	template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
	template <typename TComponent> void RemoveComponent();
	template <typename TComponent> bool HasComponent() const;
	template <typename TComponent> TComponent& GetComponent() const;

	// Hold a pointer to the entity's owner register
	class Registry* registry;
};

//*************************************************************************************
// SYSTEM
// The System processes entities that contain a specific component signature
//*************************************************************************************

class System {
private:
	Signature componentSignature;
	std::vector<Entity> entities;

public:
	System() = default;
	//virtual ~System() = default;
	~System() = default;

	void AddEntityToSystem(Entity entity);
	void RemoveEntityFromSystem(Entity entity);
	std::vector<Entity> GetSystemEntities() const;
	const Signature& GetComponentSignature() const;

	// Define the component type T that entities must have to be considered by the system
	template <typename TComponent> void RequireComponent();
};

//*************************************************************************************
// POOL CLASS
// A pool is a vector (contiguous data) of objects of type T
//*************************************************************************************

class IPool {						// base/parent class IPool
public:
	virtual ~IPool() {}
};

template <typename T>
class Pool : public IPool {
private:
	std::vector<T> data;

public:
	Pool(int size = 100) {
		data.resize(size);
	}

	virtual ~Pool() = default;

	bool isEmpty() const{
		return data.empty();
	}

	int GetSize() const{
		return data.size();
	}

	void Resize(int n) {
		data.resize(n);
	}

	void Clear() {
		data.clear();
	}

	void Add(T object) {
		data.push_back(object);
	}

	void Set(int index, T object) {
		data[index] = object;
	}

	T& Get(int index) {
		return static_cast<T&>(data[index]);
	}

	T& operator [](unsigned int index) {
		return data[index];
	}

};

//*************************************************************************************
// REGISTRY
// The registry manages the creation and destruction of entities, as well as
// adding systems and adding components to entities
//*************************************************************************************

class Registry {
private:
	// Keeping track of how many entities were added to the scene
	int numEntities = 0;

	// Vector of component pools
	// Each pool contains all the data for a certain component type
	// [vector index = component type id]
	// [pool index = entity id]
	std::vector<std::shared_ptr<IPool>> componentPools;			// default back to parent class for type

	// Vector of component signatures per entity, indicating which component is turned 'on' for a given entity
	// [Vector index = entity id]
	std::vector<Signature> entityComponentSignatures;

	// Map of active systems [index = system typeId]
	// Unordered_map can be used since we do not need to keep the elements sorted
	std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

	// Avoid creating or destroying entities in the middle of the game logic by flagging entities 
	// to be added or removed in the next registry Update()
	std::set<Entity> entitiesToBeAdded;		// Entities awaiting creation in the next Registry Update()
	std::set<Entity> entitiesToBeKilled;	// Entities awaiting destruction in the next Registry Update()

public:
	Registry() {
		Logger::Log("Registry constructor called");
	}

	~Registry() {
		Logger::Log("Registry destructor called");
	}

	// Process entities that are waiting to be added/killed
	void Update();

	// Entity Management
	Entity CreateEntity();

	// Component Management
	template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
	template <typename TComponent> void RemoveComponent(Entity entity);
	template <typename TComponent> bool HasComponent(Entity entity) const;
	template <typename TComponent> TComponent& GetComponent(Entity entity) const;
	
	// System management
	template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args);
	template <typename TSystem> void RemoveSystem();
	template <typename TSystem> bool HasSystem() const;
	template <typename TSystem> TSystem& GetSystem() const;

	// Check the component signature of an entity and add the entity to the systems
	// that are interested in it
	void AddEntityToSystems(Entity entity);
};



 // Implementing template functions in the header file

template <typename TComponent>
void System::RequireComponent() {
	const auto componentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}

template <typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args) {
	std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
	systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template <typename TSystem>
void Registry::RemoveSystem() {
	auto system = systems.find(std::type_index(typeid(TSystem)));
	systems.erase(system);
}

template <typename TSystem>
bool Registry::HasSystem() const {
	return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template <typename TSystem>
TSystem& Registry::GetSystem() const {
	auto system = systems.find(std::type_index(typeid(TSystem)));
	return *(std::static_pointer_cast<TSystem>(system->second));
}

template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	// if id > then resize
	if (componentId >= componentPools.size()) {
		componentPools.resize(componentId + 1, nullptr);
	}

	// if no position in component pool, create new pool
	if (!componentPools[componentId]) {
		std::shared_ptr <Pool<TComponent>> newComponentPool = std::make_shared <Pool<TComponent>>();
		componentPools[componentId] = newComponentPool;
	}

	// get component pool
	std::shared_ptr <Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>> (componentPools[componentId]);

	// if pool too small, resize
	if (entityId >= componentPool->GetSize()) {
		componentPool->Resize(numEntities);
	}

	// create component
	TComponent newComponent(std::forward<TArgs>(args)...);

	// set component pool position
	componentPool->Set(entityId, newComponent);

	// turn id signature on
	entityComponentSignatures[entityId].set(componentId);

	Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id " + std::to_string(entityId));
}

template <typename TComponent>
void Registry::RemoveComponent(Entity entity) {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	entityComponentSignatures[entityId].set(componentId, false);

	Logger::Log("Component id = " + std::to_string(componentId) + " was removed from entity id " + std::to_string(entityId));
}

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	return entityComponentSignatures[entityId].test(componentId);
}

template <typename TComponent>
TComponent& Registry::GetComponent(Entity entity) const {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	auto componentPool = std::static_pointer_cast<Pool<TComponent>> (componentPools[componentId]);
	return componentPool->Get(entityId);
}



template <typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs&& ...args) {
	registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args) ...);
}

template <typename TComponent>
void Entity::RemoveComponent() {
	registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent>
bool Entity::HasComponent() const {
	return registry->HasComponent<TComponent>(*this);
}

template <typename TComponent>
TComponent& Entity::GetComponent() const {
	return registry->GetComponent<TComponent>(*this);
}

#endif
