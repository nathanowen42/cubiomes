#include <atomic>
#include <chrono>
#include <deque>
#include <iostream>
#include <map>
#include <random>
#include <stdio.h>
#include <thread>
#include <utility>

#include "finders.h"

/*
 * This program outputs awesome island seeds
 * An awesome island seed is a seed in which
 * spawn is located on an island, that island is near a mushroom biome
 * and there is at least one village island nearby
 */

std::atomic<uint64_t> numFinds{0};
const auto startTime = std::chrono::steady_clock::now();

void search()
{
    const auto version = MC_1_15;
    const int64_t minIslandSizeBlocks = 500;
    const int64_t maxIslandSizeBlocks = 50000;
    const int rangeToSearch = 500;    
    const int biomesToFind[] = {mushroom_fields};

    // Allocate and initialize a stack of biome layers that reflects the biome
    // generation of Minecraft
    LayerStack layer = setupGenerator(version);

    std::random_device rd;
    std::mt19937_64 eng(rd()); 
    std::uniform_int_distribution<unsigned long long> distr;

    auto biomeFilter = setupBiomeFilter(biomesToFind, sizeof(biomesToFind)/sizeof(int));

    while(true)
    {
        int64_t seed = distr(eng);

        //std::cout << "checking " << seed << std::endl;

        // Go through the layers in the layer stack and initialize the seed
        // dependent aspects of the generator.
        applySeed(&layer, seed);

        //quick checks for ocean nearby, creates false negatives but speeds things up a lot

        bool foundOcean = false;

        for(int i = 0; i < 160; i += 16)
        {
            auto pos = Pos{0, i};

            if (isOceanic(getBiomeAtPos(layer, pos)))
            {
                foundOcean = true;
                break;
            }
        }

        if(!foundOcean)
        {
            continue;
        }

    	Pos spawnPos = getSpawn(version, &layer, nullptr, seed);

        int edgeLen = rangeToSearch * 2;
        int *cache = allocCache(&layer.layers[L_VORONOI_ZOOM_1], edgeLen, edgeLen);
        bool hasBiomes = static_cast<bool>(checkForBiomes(&layer, cache, seed, spawnPos.x - rangeToSearch, spawnPos.z - rangeToSearch, edgeLen, edgeLen, biomeFilter, 1));
        free(cache);

        if(!hasBiomes)
        {
            continue;
        }

        Pos nearestLand = findClosestLand(spawnPos, layer);
        
        if (!isIsland(nearestLand, layer, minIslandSizeBlocks, maxIslandSizeBlocks))
        {
            continue;
        }
    
        auto foundVillages = getVillagesInRange(spawnPos, layer, seed, rangeToSearch);

        if(foundVillages.empty())
        {
            continue;
        }

        uint64_t numIslandVillage = 0;

        for(auto& islandPos : foundVillages)
        {
            nearestLand = findClosestLand(islandPos, layer);
            
            if(isIsland(nearestLand, layer, minIslandSizeBlocks, maxIslandSizeBlocks))
            {
                numIslandVillage++;
            }
        }

        if(0 == numIslandVillage)
        {
            continue;
        }

        std::cout << "found seed: " << seed << " with " << numIslandVillage << " island villages" << std::endl;
        numFinds++;
        std::cout << "mean time between finds = " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - startTime).count() / numFinds << std::endl;
    }

    // Clean up.
    freeGenerator(layer);
}

int main()
{
    static constexpr int num_threads = 8;
    std::array<std::thread, num_threads> threads;

    // First initialize the global biome table 'int biomes[256]'. This sets up
    // properties such as the category and temperature of each biome.
    initBiomes();

    for(auto& t : threads)
    {
        t = std::thread{search};
    }

    for(auto& t : threads)
    {
        t.join();
    }
    
    return 0;
}

