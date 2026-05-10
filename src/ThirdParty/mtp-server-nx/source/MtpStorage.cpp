/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "MtpStorage"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <chrono>

#include "MtpDebug.h"
#include "MtpDatabase.h"
#include "MtpStorage.h"
#include "log.h"

namespace android {

namespace {

constexpr long long kFreeSpaceCacheMs = 2000;

long long nowMs() {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

} // namespace

MtpStorage::MtpStorage(MtpStorageID id, const char* filePath,
        const char* description, uint64_t reserveSpace,
        bool removable, uint64_t maxFileSize)
    :   mStorageID(id),
        mFilePath(filePath),
        mDescription(description),
        mMaxCapacity(0),
        mCachedFreeSpace(0),
        mCachedFreeSpaceMs(0),
        mMaxFileSize(maxFileSize),
        mReserveSpace(reserveSpace),
        mRemovable(removable)
{
    VLOG(2) << "MtpStorage id: " << id << " path: " << filePath;
}

MtpStorage::~MtpStorage() {
}

int MtpStorage::getType() const {
    return (mRemovable ? MTP_STORAGE_REMOVABLE_RAM :  MTP_STORAGE_FIXED_RAM);
}

int MtpStorage::getFileSystemType() const {
    return MTP_STORAGE_FILESYSTEM_HIERARCHICAL;
}

int MtpStorage::getAccessCapability() const {
    return MTP_STORAGE_READ_WRITE;
}

uint64_t MtpStorage::getMaxCapacity() {
    if (mMaxCapacity == 0) {
        struct statvfs   stat;
        if (statvfs(getPath(), &stat))
            return -1;
        mMaxCapacity = (uint64_t)stat.f_blocks * (uint64_t)stat.f_bsize;
    }
    return mMaxCapacity;
}

uint64_t MtpStorage::getFreeSpace() {
    const long long now = nowMs();
    if (mCachedFreeSpaceMs > 0 && (now - mCachedFreeSpaceMs) < kFreeSpaceCacheMs)
        return mCachedFreeSpace;

    struct statvfs   stat;
    if (statvfs(getPath(), &stat))
        return -1;
    uint64_t freeSpace = (uint64_t)stat.f_bavail * (uint64_t)stat.f_bsize;
    mCachedFreeSpace = (freeSpace > mReserveSpace ? freeSpace - mReserveSpace : 0);
    mCachedFreeSpaceMs = now;
    return mCachedFreeSpace;
}

const char* MtpStorage::getDescription() const {
    return mDescription.c_str();
}

}  // namespace android
