#ifndef CARTA_BACKEND__TILE_CACHE_H_
#define CARTA_BACKEND__TILE_CACHE_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <list>

#include "ImageData/FileLoader.h"

#if _UNIT_TEST_
#include <gtest/gtest_prod.h>
#endif

struct TileCacheKey {
    TileCacheKey() {}
    TileCacheKey(int x, int y) : x(x), y(y) {}
    
    bool operator==(const TileCacheKey &other) const { 
        return (x == other.x && y == other.y);
    }
    
    int x;
    int y;
};

namespace std {
  template <>
  struct hash<TileCacheKey>
  {
    std::size_t operator()(const TileCacheKey& k) const
    {
        return std::hash<int>()(k.x) ^ (std::hash<int>()(k.x) << 1);
    }
  };
}

class TileCache {
public:
    using Key = TileCacheKey;

    TileCache() {}
    TileCache(int capacity) : _capacity(capacity), _channel(-1), _stokes(-1) {}
    
    // This is read-only and does not lock the cache
    bool Peek(std::vector<float>& tile_data, Key key);
    
    // These functions lock the cache
    bool Get(std::vector<float>& tile_data, Key key, std::shared_ptr<carta::FileLoader> loader, std::mutex& image_mutex);
    bool GetMultiple(std::unordered_map<Key, std::vector<float>>& tiles, std::shared_ptr<carta::FileLoader> loader, std::mutex& image_mutex);
    void Reset(int channel, int stokes, int capacity = 0);
    
private:
#if _UNIT_TEST_
    FRIEND_TEST(TileCache, Create);
    FRIEND_TEST(TileCache, Reset);
    FRIEND_TEST(TileCache, Get);
#endif
    using TilePtr = std::shared_ptr<std::vector<float>>;
    using TilePair = std::pair<Key, TilePtr>;
    
    void CopyTileData(std::vector<float>& tile_data, TilePtr tile);
    TilePtr UnsafePeek(Key key);
    void Touch(Key key);
    Key ChunkKey(Key tile_key);
    bool LoadChunk(Key chunk_key, std::shared_ptr<carta::FileLoader> loader, std::mutex& image_mutex);
    
    int _channel;
    int _stokes;
    std::list<TilePair> _queue;
    std::unordered_map<Key, std::list<TilePair>::iterator> _map;
    int _capacity;
    std::mutex _tile_cache_mutex;
    
    static const int _TILE_SQ;
    static const int _CHUNK_SIZE;
    static const int _CHUNK_SQ;
};

#endif // CARTA_BACKEND__TILE_CACHE_H_
