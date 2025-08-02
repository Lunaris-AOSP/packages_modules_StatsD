/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <gtest/gtest.h>

#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include "socket/LogEventFilterUtils.h"

namespace android {
namespace os {
namespace statsd {

constexpr int kAtomIdsCount = 100;

// Helper to generate a set of atom IDs
template <typename SetType>
SetType generateAtomIds(int start, int end) {
    SetType ids;
    for (int i = start; i <= end; ++i) {
        ids.insert(i);
    }
    return ids;
}

// Test fixture for AtomIdSetManagerBase.
template <typename SetType>
class AtomIdSetManagerBaseTest : public ::testing::Test {
protected:
    using Manager = AtomIdSetManagerBase<SetType>;
    Manager manager;
};

using SetTypesToTest = ::testing::Types<std::set<int>, std::unordered_set<int>>;
TYPED_TEST_SUITE(AtomIdSetManagerBaseTest, SetTypesToTest);

TYPED_TEST(AtomIdSetManagerBaseTest, TestEmpty) {
    const auto sampleIds = generateAtomIds<TypeParam>(1, kAtomIdsCount);
    for (const auto& atomId : sampleIds) {
        EXPECT_FALSE(isAtomInSet(this->manager.getAtomIds(), atomId));
    }
}

TYPED_TEST(AtomIdSetManagerBaseTest, TestRemoveNonExistingConsumer) {
    EXPECT_FALSE(isAtomInSet(this->manager.getAtomIds(), 1));
    typename TestFixture::Manager::AtomIdSet emptyAtomIdsSet;
    this->manager.setAtomIds(emptyAtomIdsSet,
                             reinterpret_cast<typename TestFixture::Manager::ConsumerId>(0));
    EXPECT_FALSE(isAtomInSet(this->manager.getAtomIds(), 1));
}

TYPED_TEST(AtomIdSetManagerBaseTest, TestSingleConsumer) {
    auto filterIds = generateAtomIds<TypeParam>(1, kAtomIdsCount);
    this->manager.setAtomIds(filterIds,
                             reinterpret_cast<typename TestFixture::Manager::ConsumerId>(0));

    for (int i = 1; i <= kAtomIdsCount; ++i) {
        EXPECT_TRUE(isAtomInSet(this->manager.getAtomIds(), i));
    }
    for (int i = kAtomIdsCount + 1; i <= kAtomIdsCount * 2; ++i) {
        EXPECT_FALSE(isAtomInSet(this->manager.getAtomIds(), i));
    }
}

TYPED_TEST(AtomIdSetManagerBaseTest, TestMultipleConsumersWithOverlap) {
    auto filterIds1 = generateAtomIds<TypeParam>(1, kAtomIdsCount);
    auto filterIds2 = generateAtomIds<TypeParam>(kAtomIdsCount / 2, kAtomIdsCount * 3 / 2);
    this->manager.setAtomIds(filterIds1,
                             reinterpret_cast<typename TestFixture::Manager::ConsumerId>(0));
    this->manager.setAtomIds(filterIds2,
                             reinterpret_cast<typename TestFixture::Manager::ConsumerId>(1));

    for (int i = 1; i <= kAtomIdsCount * 3 / 2; ++i) {
        EXPECT_TRUE(isAtomInSet(this->manager.getAtomIds(), i));
    }
    EXPECT_FALSE(isAtomInSet(this->manager.getAtomIds(), kAtomIdsCount * 3 / 2 + 1));
}

TYPED_TEST(AtomIdSetManagerBaseTest, TestMultipleConsumersRemoveOne) {
    auto filterIds1 = generateAtomIds<TypeParam>(1, kAtomIdsCount);
    auto filterIds2 = generateAtomIds<TypeParam>(kAtomIdsCount + 1, kAtomIdsCount * 2);
    this->manager.setAtomIds(filterIds1,
                             reinterpret_cast<typename TestFixture::Manager::ConsumerId>(0));
    this->manager.setAtomIds(filterIds2,
                             reinterpret_cast<typename TestFixture::Manager::ConsumerId>(1));

    typename TestFixture::Manager::AtomIdSet emptySet;
    this->manager.setAtomIds(emptySet,
                             reinterpret_cast<typename TestFixture::Manager::ConsumerId>(1));

    for (int i = 1; i <= kAtomIdsCount * 2; ++i) {
        bool expectedInUse = (i <= kAtomIdsCount);
        EXPECT_EQ(expectedInUse, isAtomInSet(this->manager.getAtomIds(), i));
    }
}

}  // namespace statsd
}  // namespace os
}  // namespace android