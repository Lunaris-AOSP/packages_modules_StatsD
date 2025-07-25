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

#pragma once

#include <com_android_os_statsd_flags.h>
#include <gtest/gtest_prod.h>

#include <mutex>

#include "LogEventFilterUtils.h"
#include "socket/AtomsInUseChangeListener.h"

namespace android {
namespace os {
namespace statsd {

namespace flags = com::android::os::statsd::flags;

/**
 * Configures socket logging control based on list af atom ids in use
 */
class SocketLogEventControl : public AtomsInUseChangeListener {
public:
    virtual ~SocketLogEventControl() = default;

    void setControlEnabled(bool isEnabled) {
        if (!isActive()) {
            return;
        }

        if (isEnabled) {
            std::lock_guard lock(mTagIdsMutex);
            setLoggingConfig(mAtomIdSetManager.getAtomIds());
        } else {
            // to allow clients to log any atom
            resetConfig();
        }
    }

    void setAtomIds(AtomIdSet tagIds, ConsumerId consumer) override {
        if (!isActive()) {
            return;
        }

        std::lock_guard lock(mTagIdsMutex);
        mAtomIdSetManager.setAtomIds(tagIds, consumer);
        setLoggingConfig(mAtomIdSetManager.getAtomIds());
    }

private:
    mutable std::mutex mTagIdsMutex;
    mutable AtomIdSetManager mAtomIdSetManager;

    void setLoggingConfig(const AtomIdSet& atomsInUse) {
        // TODO (b/407064406): sets system property & creates atom list file
    }

    void resetConfig() {
        // TODO: resets system property & removes atom list file
    }

    static bool isActive() {
        static const bool featureActive = isAtLeastB() && flags::logging_control_enabled();
        return featureActive;
    }
};

}  // namespace statsd
}  // namespace os
}  // namespace android
