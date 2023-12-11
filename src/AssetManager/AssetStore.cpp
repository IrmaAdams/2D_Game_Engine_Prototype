#include "./AssetStore.h"
#include "../Logger.h"

AssetStore::AssetStore() {
	Logger::Log("AssetStore constructor called");
}

AssetStore::~AssetStore() {
	Logger::Log("AssetStore destructor called");
}

void AssetStore::ClearAssets() {
	// todo -
}

void AssetStore::AddTexture(const std::string& assetId, const std::string& filePath) {
	// todo -
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId) const {
	// todo - 
}