#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../AssetManager/AssetStore.h"
#include <SDL.h>
#include <algorithm>

class RenderSystem : public System {
public:
	RenderSystem() {
		RequireComponent<TransformComponent>();
		RequireComponent<SpriteComponent>();
	}

	void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore) {
		// todo - sort all the entities of the render system by z index
		// sorting all entities every frame is a red flag - performance heavy
		
		// Create a vector with both Sprite and Transform component of all entities
		struct RenderableEntity {															// create a helper struct
			TransformComponent transformComponent;
			SpriteComponent spriteComponent;
		};
		std::vector<RenderableEntity> renderableEntities;									// create a vector of renderable entities
		for (auto entity : GetSystemEntities()) {											// loop all entities of the system
			RenderableEntity renderableEntity;												// for each entity, create a renderable equivalent
			renderableEntity.spriteComponent = entity.GetComponent<SpriteComponent>();
			renderableEntity.transformComponent = entity.GetComponent<TransformComponent>();
			renderableEntities.emplace_back(renderableEntity);								// add the renderable entity to the vector
		}

		// Sort the vector by the z-index value
		std::sort(renderableEntities.begin(), renderableEntities.end(), [](const RenderableEntity& a, const RenderableEntity& b) {
			return a.spriteComponent.zIndex < b.spriteComponent.zIndex;						// use sort() to iterate zIndex from begin to end
		});


		// Loop all entities that the system is interested in
		for (auto entity : renderableEntities) {											// loop through the vector for renderable entities
			const auto transform = entity.transformComponent;
			const auto sprite = entity.spriteComponent;

			// Set the source rectangle of original sprite texture
			SDL_Rect srcRect = sprite.srcRect;

			// Set the destination rectangle with the x,y position to be rendered
			SDL_Rect dstRect = {
				static_cast<int>(transform.position.x),
				static_cast<int>(transform.position.y),
				static_cast<int>(sprite.width * transform.scale.x),
				static_cast<int>(sprite.height * transform.scale.y)
			};

			// https://wiki.libsdl.org/SDL2/SDL_RenderCopyEx
			SDL_RenderCopyEx(
				renderer,
				assetStore->GetTexture(sprite.assetId),
				&srcRect,
				&dstRect,
				transform.rotation,
				NULL,
				SDL_FLIP_NONE
				);

			// todo - draw png texture
		}
	}
};

#endif
