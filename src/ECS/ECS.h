#ifndef ECS_H
#define ECS_H

#include <vector>
#include<bitset>
#include <set>
#include <unordered_map>
#include <typeindex>


const unsigned int MAX_COMPONENTS = 32;

// SIGNATURE
// Use a bitset (1s and 0s) to keep track of which components an entity has,
// and also helps keep track of which entities a system is interested in.

typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent {
protected:
	static int nextId;
};

// used to assign a unique id to a component taype
template <typename T>

class Component: public IComponent {
	// returns the unique id of Component<T>
	static int GetId() {
		static auto id = nextId++;
		return id;
	}
};

class Entity {
private:
	int id;

public:
	Entity(int id) : id(id) {};		// initialize member variables directly
	Entity(const Entity& entity) = default;
	int GetId() const;

	// Operator overloading for entities:
	Entity& operator = (const Entity& other) = default;
	bool operator == (const Entity& other) const { return id == other.id; }
	bool operator != (const Entity& other) const { return id != other.id; }
	bool operator > (const Entity& other) const { return id > other.id; }
	bool operator < (const Entity& other) const { return id < other.id; }
};


// SYSTEM
// The System processes entities that contain a specific component signature

class System {
private:
	Signature componentSignature;
	std::vector<Entity> entities;

public:
	System() = default;
	virtual ~System() = default;

	void AddEntityToSystem(Entity entity);
	void RemoveEntityFromSystem(Entity entity);
	std::vector<Entity> GetSystemEntities() const;
	const Signature& GetComponentSignature() const;

	// Define the component type T that entities must have to be considered by the system
	template <typename TComponent> void RequireComponent();
};

// POOL CLASS
// A pool is a vector (contiguous data) of objects of type T

class IPool {						// base class IPool
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

	bool isEmpty() {
		return data.empty;
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

// REGISTRY
// The registry manages the creation and destruction of entities, as well as
// adding systems and adding components to entities

class Registry {
private:
	// Keeping track of how many entities were added to the scene
	int numEntities = 0;

	// Avoid creating or destroying entities in the middle of the game logic by flagging entities 
	// to be added or removed in the next registry Update()
	std::set<Entity> entitiesToBeAdded;		// Entities awaiting creation in the next Registry Update()
	std::set<Entity> entitiesToBeKilled;	// Entities awaiting destruction in the next Registry Update()

	// Vector of component pools
	// Each pool contains all the data for a certain component type
	// [vector index = component type id]
	// [pool index = entity id]
	std::vector<IPool*> componentPools;			// default back to parent class for type

	// Vector of component signatures per entity, indicating which component is turned 'on' for a given entity
	// [Vector index = entity id]
	std::vector<Signature> entityComponentSignatures;


	// Map of active systems [index = system typeId]
	// Unordered_map can be used since we do not need to keep the elements sored
	std::unordered_map<std::type_index, System*> systems;

public:
	Registry() = default;

	// Management of entities
	void Update();
	Entity CreateEntity();

	//todo: AddComponent<T>();
	
	void AddEntityToSystem(Entity entity);	// add entity to the list of entities inside the system only if 
											// the component signature of an entity matches the requirement components of the system

	//void KillEntity(Entity entity);
	//void AddSystem();
	//void AddComponent();
	//void RemoveComponent();
};

template <typename TComponent>
void System::RequireComponent() {
	const auto componentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}

#endif
