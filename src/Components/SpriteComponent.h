#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H

#include <string>

struct SpriteComponent {
	std::string assetId;
	int width;
	int height;

	// Initialize component using constructor method
	SpriteComponent(int width = 0, int height = 0) {
		this->width = width;
		this->height = height;
	}
};


#endif
