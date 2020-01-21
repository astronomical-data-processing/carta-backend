#include <gtest/gtest.h>

#include "../ImageData/FileLoader.h"
#include "../ImageData/CartaHdf5Image.h"
#include "../TileCache.h"

namespace carta {

class DummyLoader : public FileLoader {
public:
    DummyLoader(const std::string& filename) {}
    void OpenFile(const std::string& hdu) override {}
    bool HasData(FileInfo::Data ds) const override { return true; }
    ImageRef GetImage() override { return _image.get(); }
    bool GetChunk(std::vector<float>& data, int min_x, int min_y, int channel, int stokes, std::mutex& image_mutex) override;
private:
    std::unique_ptr<CartaHdf5Image> _image;    
};

bool DummyLoader::GetChunk(std::vector<float>& data, int min_x, int min_y, int channel, int stokes, std::mutex& image_mutex) {
    data.resize(256 * 256);
    for (int i = 0; i < data.size(); i++) {
        data[i] = 0.5;
    }
}

} // namespace carta

TEST(TileCache, Create) {
    TileCache cache(10);
    ASSERT_EQ(cache._capacity, 10);
    ASSERT_EQ(cache._channel, -1);
    ASSERT_EQ(cache._stokes, -1);
}

TEST(TileCache, Reset) {
    TileCache cache(10);
    
    cache.Reset(5, 3);
    ASSERT_EQ(cache._capacity, 10);
    ASSERT_EQ(cache._channel, 5);
    ASSERT_EQ(cache._stokes, 3);
    
    cache.Reset(10, 2, 20);
    ASSERT_EQ(cache._capacity, 20);
    ASSERT_EQ(cache._channel, 10);
    ASSERT_EQ(cache._stokes, 2);
}

TEST(TileCache, Get) {
    std::shared_ptr<carta::FileLoader> loader = std::make_shared<carta::DummyLoader>("dummy filename");
    TileCache cache(10);
    std::vector<float> tile_data;
    std::mutex dummy_mutex;
    cache.Get(tile_data, TileCache::Key(0, 256), loader, dummy_mutex);
    ASSERT_EQ(cache._queue.size(), 4);
}
