#ifndef ECS_H
#define ECS_H

#include<bitset>
#include <vector>

const unsigned int MAX_COMPONENTS = 32;

//////////////////////////////////////////////////////////////////////////////
// SIGNATURE
//////////////////////////////////////////////////////////////////////////////
// Use a bitset (1s and 0s) to keep track of which components an entity has,
// and also helps keep track of which entities a system is interested in.
//////////////////////////////////////////////////////////////////////////////

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
	int GetId() const;
};

//////////////////////////////////////////////////////////////////////////////
// SYSTEM
//////////////////////////////////////////////////////////////////////////////
// The System processes entities that contain a specific component signature
//////////////////////////////////////////////////////////////////////////////

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
	Signature& GetComponentSignature() const;

	// Define the component type T that entities must have to be considered by the system
	template <typename T> void RequireComponent();
};

class Registry {

};

template <typename T>
void System::RequireComponent() {
	const auto componentId = Component<T>::GetId();
	componentSignature.set(componentId)
}

#endif
