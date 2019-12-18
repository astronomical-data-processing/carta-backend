#ifndef CARTA_BACKEND__TILE_CACHE_H_
#define CARTA_BACKEND__TILE_CACHE_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <list>

#include "ImageData/FileLoader.h"

struct CachedTileKey {
    CachedTileKey() {}
    CachedTileKey(int32_t x, int32_t y) : x(x), y(y) {}
    
    bool operator==(const CachedTileKey &other) const { 
        return (x == other.x && y == other.y);
    }
    
    int32_t x;
    int32_t y;
};

struct CachedTileKeyHash {
  std::size_t operator()(const CachedTileKey& k) const
  {
      return std::hash<int32_t>()(k.x) ^ (std::hash<int32_t>()(k.x) << 1);
  }
};

class TileCache {
using CachedTilePtr = std::shared_ptr<std::vector<float>>;
using CachedTilePair = std::pair<CachedTileKey, CachedTilePtr>;
public:
    TileCache() {}
    TileCache(int capacity) : _capacity(capacity) {}
    
    CachedTilePtr Peek(CachedTileKey key);
    CachedTilePtr Get(CachedTileKey key, const carta::FileLoader* loader);
    void GetMultiple(std::unordered_map<CachedTileKey, CachedTilePtr>& tiles, std::vector<CachedTileKey> keys, const carta::FileLoader* loader);
    
    void reset(int32_t channel, int32_t stokes);
    
private:
    CachedTilePtr UnsafePeek(CachedTileKey key);
    void Touch(CachedTileKey key);
    CachedTilePtr Load(CachedTileKey key, const carta::FileLoader* loader, std::mutex image_mutex);
    
    int32_t _channel;
    int32_t _stokes;
    std::list<CachedTilePair> _queue;
    std::unordered_map<CachedTileKey, std::list<CachedTilePair>::iterator, CachedTileKeyHash> _map;
    int _capacity;
    std::mutex _tile_cache_mutex; // maybe change to tbb mutex
};

#endif // CARTA_BACKEND__TILE_CACHE_H_