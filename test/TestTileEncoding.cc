#include <chrono>
#include <random>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "../Tile.h"

using namespace std;

TEST(TileEncoding, InvalidInput) {
    // Layer can be from 0 to 12
    ASSERT_EQ(Tile::Encode(0, 0, -1), -1);
    ASSERT_EQ(Tile::Encode(0, 0, 13), -1);
    // X and Y coordinates from 0 to 4095
    ASSERT_EQ(Tile::Encode(-1, 0, 12), -1);
    ASSERT_EQ(Tile::Encode(4096, 0, 12), -1);
    ASSERT_EQ(Tile::Encode(0, -1, 12), -1);
    ASSERT_EQ(Tile::Encode(0, 4096, 12), -1);
}

TEST(TileEncoding, OutOfBounds) {
    // X and Y coordinates from 0 to 2^layer -1
    ASSERT_EQ(Tile::Encode(0, 1024, 10), -1);
    ASSERT_EQ(Tile::Encode(0, 256, 8), -1);
    ASSERT_EQ(Tile::Encode(0, 4, 2), -1);
}

TEST(TileEncoding, RoundTrip) {
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<> layer_random(0, 12);
    uniform_real_distribution<float> float_random(0, 1);

    for (auto i = 0; i < 10000; i++) {
        int32_t layer = layer_random(mt);
        int32_t layer_width = 1 << layer;
        int32_t x = floor(float_random(mt) * layer_width);
        int32_t y = floor(float_random(mt) * layer_width);

        int32_t encoded_value = Tile::Encode(x, y, layer);
        auto tile = Tile::Decode(encoded_value);
        ASSERT_EQ(tile.x, x);
        ASSERT_EQ(tile.y, y);
        ASSERT_EQ(tile.layer, layer);
    }
}

TEST(TileEncoding, PerformanceTestEncoding) {
    int32_t layer = 12;
    int64_t encoded_val = 0;
    auto t_start = chrono::high_resolution_clock::now();
    for (auto i = 0; i < 1000; i++) {
        for (auto j = 0; j < 1000; j++) {
            encoded_val += Tile::Encode(i, j, layer);
        }
    }
    auto t_end = chrono::high_resolution_clock::now();
    float dt = chrono::duration_cast<chrono::microseconds>(t_end - t_start).count() / 1000.0f;
    fmt::print("Encoded 1M coordinates in {} ms", dt);
    ASSERT_EQ(encoded_val, 203373043500000);
    ASSERT_LT(dt, 2.0f);
}

TEST(TileEncoding, PerformanceTestDecoding) {
    int32_t layer = 12;
    int32_t layer_width = 1 << layer;
    int64_t encoded_val = 0;
    int64_t counter = 0;

    auto t_start = chrono::high_resolution_clock::now();

    for (auto i = 0; i < 1000; i++) {
        for (auto j = 0; j < 1000; j++) {
            counter += Tile::Decode(encoded_val).x;
            encoded_val++;
        }
        encoded_val += layer_width;
    }

    auto t_end = chrono::high_resolution_clock::now();
    float dt = chrono::duration_cast<chrono::microseconds>(t_end - t_start).count() / 1000.0f;
    fmt::print("Decoded 1M coordinates in {} ms", dt);
    ASSERT_EQ(counter, 2046486240);
    ASSERT_LT(dt, 2.0f);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}