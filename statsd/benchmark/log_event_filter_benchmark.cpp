/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <random>
#include <set>
#include <unordered_set>
#include <vector>

#include "benchmark/benchmark.h"
#include "socket/LogEventFilterUtils.h"

namespace android {
namespace os {
namespace statsd {

namespace {

constexpr int kAtomIdsCount = 500;         //  Filter size setup
constexpr int kAtomIdsSampleCount = 3000;  //  Queries number

using AtomIdType = int32_t;

std::vector<AtomIdType> generateSampleAtomIdsList() {
    std::vector<AtomIdType> atomIds(kAtomIdsSampleCount);

    std::default_random_engine generator;

    // Get atoms ids which are not in the filter to test behavior when set is searched for an
    // an absent key
    // Expected atoms ids are in a range 1..3000, random & evenly distributes
    std::uniform_int_distribution<AtomIdType> distribution(1, kAtomIdsSampleCount);

    for (int i = 0; i < kAtomIdsSampleCount; ++i) {
        atomIds[i] = distribution(generator);
    }

    return atomIds;
}

template <typename T>
T generateAtomIds() {
    T atomIds;

    std::default_random_engine generator;
    std::uniform_int_distribution<AtomIdType> distribution(1, kAtomIdsCount);

    for (int i = 0; i < kAtomIdsCount; ++i) {
        atomIds.insert(distribution(generator));
    }

    return atomIds;
}

// Used to setup filter
const std::set<AtomIdType> kAtomIdsSet = generateAtomIds<std::set<AtomIdType>>();
const std::unordered_set<AtomIdType> kAtomIdsUnorderedSet =
        generateAtomIds<std::unordered_set<AtomIdType>>();

const std::set<AtomIdType> kAtomIdsSet2 = generateAtomIds<std::set<AtomIdType>>();
const std::unordered_set<AtomIdType> kAtomIdsUnorderedSet2 =
        generateAtomIds<std::unordered_set<AtomIdType>>();

const std::set<AtomIdType> kAtomIdsSet3 = generateAtomIds<std::set<AtomIdType>>();
const std::unordered_set<AtomIdType> kAtomIdsUnorderedSet3 =
        generateAtomIds<std::unordered_set<AtomIdType>>();

const std::set<AtomIdType> kAtomIdsSet4 = generateAtomIds<std::set<AtomIdType>>();
const std::unordered_set<AtomIdType> kAtomIdsUnorderedSet4 =
        generateAtomIds<std::unordered_set<AtomIdType>>();

// Used to perform sample quieries
const std::vector<AtomIdType> kSampleIdsList = generateSampleAtomIdsList();

}  // namespace

static void BM_LogEventFilterUnorderedSet(benchmark::State& state) {
    while (state.KeepRunning()) {
        using AtomIdSetManager = AtomIdSetManagerBase<std::unordered_set<AtomIdType>>;
        AtomIdSetManager setMgr;

        // populate
        setMgr.setAtomIds(kAtomIdsUnorderedSet, nullptr);
        // many fetches
        for (const auto& atomId : kSampleIdsList) {
            benchmark::DoNotOptimize(isAtomInSet(setMgr.getAtomIds(), atomId));
        }
    }
}
BENCHMARK(BM_LogEventFilterUnorderedSet);

static void BM_LogEventFilterUnorderedSet2Consumers(benchmark::State& state) {
    while (state.KeepRunning()) {
        using AtomIdSetManager = AtomIdSetManagerBase<std::unordered_set<AtomIdType>>;
        AtomIdSetManager setMgr;

        // populate
        setMgr.setAtomIds(kAtomIdsUnorderedSet, &kAtomIdsUnorderedSet);
        setMgr.setAtomIds(kAtomIdsUnorderedSet2, &kAtomIdsUnorderedSet2);
        setMgr.setAtomIds(kAtomIdsUnorderedSet3, &kAtomIdsUnorderedSet);
        setMgr.setAtomIds(kAtomIdsUnorderedSet4, &kAtomIdsUnorderedSet2);
        // many fetches
        for (const auto& atomId : kSampleIdsList) {
            benchmark::DoNotOptimize(isAtomInSet(setMgr.getAtomIds(), atomId));
        }
    }
}
BENCHMARK(BM_LogEventFilterUnorderedSet2Consumers);

static void BM_LogEventFilterSet(benchmark::State& state) {
    while (state.KeepRunning()) {
        using AtomIdSetManager = AtomIdSetManagerBase<std::set<AtomIdType>>;
        AtomIdSetManager setMgr;

        // populate
        setMgr.setAtomIds(kAtomIdsSet, nullptr);
        // many fetches
        for (const auto& atomId : kSampleIdsList) {
            benchmark::DoNotOptimize(isAtomInSet(setMgr.getAtomIds(), atomId));
        }
    }
}
BENCHMARK(BM_LogEventFilterSet);

static void BM_LogEventFilterSet2Consumers(benchmark::State& state) {
    while (state.KeepRunning()) {
        using AtomIdSetManager = AtomIdSetManagerBase<std::set<AtomIdType>>;
        AtomIdSetManager setMgr;

        // populate
        setMgr.setAtomIds(kAtomIdsSet, &kAtomIdsSet);
        setMgr.setAtomIds(kAtomIdsSet2, &kAtomIdsSet2);
        setMgr.setAtomIds(kAtomIdsSet3, &kAtomIdsSet);
        setMgr.setAtomIds(kAtomIdsSet4, &kAtomIdsSet2);
        // many fetches
        for (const auto& atomId : kSampleIdsList) {
            benchmark::DoNotOptimize(isAtomInSet(setMgr.getAtomIds(), atomId));
        }
    }
}
BENCHMARK(BM_LogEventFilterSet2Consumers);

}  //  namespace statsd
}  //  namespace os
}  //  namespace android
