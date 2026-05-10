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

#include <iomanip>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <limits>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <malloc.h>

#include <switch.h>

#define LOG_TAG "MtpServer"

#include "MtpDebug.h"
#include "MtpDatabase.h"
#include "MtpObjectInfo.h"
#include "MtpProperty.h"
#include "MtpServer.h"
#include "MtpStorage.h"
#include "MtpStringBuffer.h"

#include "log.h"

namespace android {

static const MtpOperationCode kSupportedOperationCodes[] = {
    MTP_OPERATION_GET_DEVICE_INFO,
    MTP_OPERATION_OPEN_SESSION,
    MTP_OPERATION_CLOSE_SESSION,
    MTP_OPERATION_GET_STORAGE_IDS,
    MTP_OPERATION_GET_STORAGE_INFO,
    MTP_OPERATION_GET_NUM_OBJECTS,
    MTP_OPERATION_GET_OBJECT_HANDLES,
    MTP_OPERATION_GET_OBJECT_INFO,
    MTP_OPERATION_GET_OBJECT,
    MTP_OPERATION_DELETE_OBJECT,
    MTP_OPERATION_GET_PARTIAL_OBJECT,
    MTP_OPERATION_SEND_OBJECT_INFO,
    MTP_OPERATION_SEND_OBJECT,
//    MTP_OPERATION_INITIATE_CAPTURE,
//    MTP_OPERATION_FORMAT_STORE,
//    MTP_OPERATION_RESET_DEVICE,
//    MTP_OPERATION_SELF_TEST,
//    MTP_OPERATION_SET_OBJECT_PROTECTION,
//    MTP_OPERATION_POWER_DOWN,
    MTP_OPERATION_GET_DEVICE_PROP_DESC,
    MTP_OPERATION_GET_DEVICE_PROP_VALUE,
    MTP_OPERATION_SET_DEVICE_PROP_VALUE,
    MTP_OPERATION_RESET_DEVICE_PROP_VALUE,
//    MTP_OPERATION_TERMINATE_OPEN_CAPTURE,
    MTP_OPERATION_MOVE_OBJECT,
//    MTP_OPERATION_COPY_OBJECT,
//    MTP_OPERATION_INITIATE_OPEN_CAPTURE,
    MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED,
    MTP_OPERATION_GET_OBJECT_PROP_DESC,
    MTP_OPERATION_GET_OBJECT_PROP_VALUE,
    MTP_OPERATION_SET_OBJECT_PROP_VALUE,
    MTP_OPERATION_GET_OBJECT_PROP_LIST,
//    MTP_OPERATION_SET_OBJECT_PROP_LIST,
//    MTP_OPERATION_GET_INTERDEPENDENT_PROP_DESC,
//    MTP_OPERATION_SEND_OBJECT_PROP_LIST,
//    MTP_OPERATION_GET_OBJECT_REFERENCES,
//    MTP_OPERATION_SET_OBJECT_REFERENCES,
//    MTP_OPERATION_SKIP,
    // Android extension for direct file IO
    MTP_OPERATION_GET_PARTIAL_OBJECT_64,
    MTP_OPERATION_SEND_PARTIAL_OBJECT,
    MTP_OPERATION_TRUNCATE_OBJECT,
    MTP_OPERATION_BEGIN_EDIT_OBJECT,
    MTP_OPERATION_END_EDIT_OBJECT,
};

static constexpr size_t kMtpTransferChunkSize = 1024 * 1024;
// Keep the first SendObject read tiny. Windows can start the data phase slowly
// and asking for too much here leaves Explorer waiting at 0% on some transfers.
static constexpr uint32_t kMtpInitialObjectReadSize = 512;
static constexpr uint64_t kMtpDataPhaseTimeoutNs = 10ULL * 1000ULL * 1000ULL * 1000ULL;

static unsigned char *getReusableTransferBuffer(size_t size)
{
    static unsigned char *buffer = nullptr;
    static size_t bufferSize = 0;
    if (buffer == nullptr || bufferSize < size) {
        if (buffer != nullptr)
            free(buffer);
        buffer = (unsigned char*)memalign(0x1000, size);
        bufferSize = (buffer != nullptr) ? size : 0;
    }
    return buffer;
}

static bool isBlockedReadPath(const char* path) {
    if (path == nullptr) {
        return false;
    }

    std::string lower(path);
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    static const char* kBlockedExts[] = {
        ".nro", ".nso", ".exe", ".dll", ".com", ".bat", ".cmd", ".scr", ".msi", ".lnk", ".js", ".vbs"
    };

    for (const char* ext : kBlockedExts) {
        const size_t extLen = std::strlen(ext);
        if (lower.size() >= extLen && lower.compare(lower.size() - extLen, extLen, ext) == 0) {
            return true;
        }
    }
    return false;
}

static const MtpEventCode kSupportedEventCodes[] = {
    MTP_EVENT_OBJECT_ADDED,
    MTP_EVENT_OBJECT_REMOVED,
    MTP_EVENT_STORE_ADDED,
    MTP_EVENT_STORE_REMOVED,
    MTP_EVENT_OBJECT_INFO_CHANGED,
    MTP_EVENT_OBJECT_PROP_CHANGED,
};

MtpServer::MtpServer(USBMtpInterface* usb, MtpDatabase* database, bool ptp,
                    int fileGroup, int filePerm, int directoryPerm)
    :   mUSB(usb),
        mDatabase(database),
        mRunning(false),
        mStopRequested(false),
        mPtp(ptp),
        mFileGroup(fileGroup),
        mFilePermission(filePerm),
        mDirectoryPermission(directoryPerm),
        mSessionID(0),
        mSessionOpen(false),
        mSendObjectHandle(kInvalidObjectHandle),
        mSendObjectFormat(0),
        mSendObjectFileSize(0)
{
}

MtpServer::~MtpServer() {
}

void MtpServer::addStorage(MtpStorage* storage) {
    MtpAutolock autoLock(mMutex);

    mStorages.push_back(storage);
    sendStoreAdded(storage->getStorageID());
}

void MtpServer::removeStorage(MtpStorage* storage) {
    MtpAutolock autoLock(mMutex);

    for (int i = 0; i < mStorages.size(); i++) {
        if (mStorages[i] == storage) {
            mStorages.erase(mStorages.begin()+i);
            sendStoreRemoved(storage->getStorageID());
            break;
        }
    }
}

MtpStorage* MtpServer::getStorage(MtpStorageID id) {
    if (id == 0)
        return mStorages[0];
    for (int i = 0; i < mStorages.size(); i++) {
        MtpStorage* storage = mStorages[i];
        if (storage->getStorageID() == id)
            return storage;
    }
    return NULL;
}

bool MtpServer::hasStorage(MtpStorageID id) {
    if (id == 0 || id == 0xFFFFFFFF)
        return mStorages.size() > 0;
    return (getStorage(id) != NULL);
}

void MtpServer::stop() {
    mStopRequested.store(true);
    mRunning.store(false);
}

void MtpServer::run() {
    USBMtpInterface* usb = mUSB;

    VLOG(1) << "MtpServer::run";

    if (mStopRequested.load()) {
        mRunning.store(false);
        return;
    }

    bool wasConfigured = false;
    auto refreshUsbState = [&wasConfigured]() {
        UsbState st = UsbState_Detached;
        if (R_SUCCEEDED(usbDsGetState(&st))) {
            if (st == UsbState_Configured) {
                wasConfigured = true;
            }
            return st;
        }
        return UsbState_Detached;
    };
    auto shouldExitAfterIoError = [&]() {
        if (!mRunning.load() || mStopRequested.load()) {
            return true;
        }
        UsbState st = refreshUsbState();
        return wasConfigured && st != UsbState_Configured;
    };

    mRunning.store(true);
    while (mRunning.load() && !mStopRequested.load()) {
        refreshUsbState();

        int ret = mRequest.read(usb);
        if (ret < 0) {
            VLOG(2) << "request read returned " << ret;
            if (shouldExitAfterIoError()) {
                break;
            }
            svcSleepThread(20'000'000); // 20ms backoff
            continue;
        }
        MtpOperationCode operation = mRequest.getOperationCode();
        MtpTransactionID transaction = mRequest.getTransactionID();

        VLOG(2) << "operation: " << MtpDebug::getOperationCodeName(operation);
        mRequest.dump();

        // FIXME need to generalize this
        bool dataIn = (operation == MTP_OPERATION_SEND_OBJECT_INFO
                    || operation == MTP_OPERATION_SET_OBJECT_REFERENCES
                    || operation == MTP_OPERATION_SET_OBJECT_PROP_VALUE
                    || operation == MTP_OPERATION_SET_DEVICE_PROP_VALUE);
        if (dataIn) {
            int ret = mData.readWithTimeout(usb, kMtpDataPhaseTimeoutNs);
            if (ret < 0) {
                VLOG(2) << "data read returned " << ret;
                continue;
            }
            VLOG(2) << "received data:";
            mData.dump();
        } else {
            mData.reset();
        }

        if (handleRequest()) {
            if (!dataIn && mData.hasData()) {
                mData.setOperationCode(operation);
                mData.setTransactionID(transaction);
                VLOG(2) << "sending data:";
                mData.dump();
                ret = mData.write(usb);
                if (ret < 0) {
                    VLOG(2) << "request write returned " << ret;
                    if (shouldExitAfterIoError()) {
                        break;
                    }
                    continue;
                }
            }

            mResponse.setTransactionID(transaction);
            VLOG(2) << "sending response "
                    << std::hex << mResponse.getResponseCode() << std::dec;
            ret = mResponse.write(usb);
            mResponse.dump();
            if (ret < 0) {
                VLOG(2) << "request write returned " << ret;
                if (shouldExitAfterIoError()) {
                    break;
                }
                continue;
            }
        } else {
            VLOG(2) << "skipping response";
        }
    }

    // commit any open edits
    int count = mObjectEditList.size();
    for (int i = 0; i < count; i++) {
        ObjectEdit* edit = mObjectEditList[i];
        commitEdit(edit);
        delete edit;
    }
    mObjectEditList.clear();

    if (mSessionOpen)
        mDatabase->sessionEnded();
    mUSB = NULL;
    mRunning.store(false);
}

void MtpServer::sendObjectAdded(MtpObjectHandle handle) {
    VLOG(1) << "sendObjectAdded " << handle;
    sendEvent(MTP_EVENT_OBJECT_ADDED, handle, 0, 0);
}

void MtpServer::sendObjectRemoved(MtpObjectHandle handle) {
    VLOG(1) << "sendObjectRemoved " << handle;
    sendEvent(MTP_EVENT_OBJECT_REMOVED, handle, 0, 0);
}

void MtpServer::sendObjectInfoChanged(MtpObjectHandle handle) {
    VLOG(1) << "sendObjectInfoChanged " << handle;
    sendEvent(MTP_EVENT_OBJECT_INFO_CHANGED, handle, 0, 0);
}

void MtpServer::sendObjectPropChanged(MtpObjectHandle handle,
                                      MtpObjectProperty prop) {
    VLOG(1) << "sendObjectPropChanged " << handle << " " << prop;
    sendEvent(MTP_EVENT_OBJECT_PROP_CHANGED, handle, prop, 0);
}

void MtpServer::sendStoreAdded(MtpStorageID id) {
    VLOG(1) << "sendStoreAdded " << std::hex << id << std::dec;
    sendEvent(MTP_EVENT_STORE_ADDED, id, 0, 0);
}

void MtpServer::sendStoreRemoved(MtpStorageID id) {
    VLOG(1) << "sendStoreRemoved " << std::hex << id << std::dec;
    sendEvent(MTP_EVENT_STORE_REMOVED, id, 0, 0);
}

void MtpServer::sendEvent(MtpEventCode code,
                          uint32_t param1,
                          uint32_t param2,
                          uint32_t param3) {
    if (mSessionOpen) {
        mEvent.setEventCode(code);
        mEvent.setTransactionID(mRequest.getTransactionID());
        mEvent.setParameter(1, param1);
        mEvent.setParameter(2, param2);
        mEvent.setParameter(3, param3);
        int ret = mEvent.write(mUSB);
        VLOG(2) << "mEvent.write returned " << ret;
    }
}

void MtpServer::addEditObject(MtpObjectHandle handle, MtpString& path,
        uint64_t size, MtpObjectFormat format, int fd) {
    ObjectEdit*  edit = new ObjectEdit(handle, path, size, format, fd);
    mObjectEditList.push_back(edit);
}

MtpServer::ObjectEdit* MtpServer::getEditObject(MtpObjectHandle handle) {
    int count = mObjectEditList.size();
    for (int i = 0; i < count; i++) {
        ObjectEdit* edit = mObjectEditList[i];
        if (edit->mHandle == handle) return edit;
    }
    return NULL;
}

void MtpServer::removeEditObject(MtpObjectHandle handle) {
    int count = mObjectEditList.size();
    for (int i = 0; i < count; i++) {
        ObjectEdit* edit = mObjectEditList[i];
        if (edit->mHandle == handle) {
            delete edit;
            mObjectEditList.erase(mObjectEditList.begin() + i);
            return;
        }
    }
    LOG(ERROR) << "ObjectEdit not found in removeEditObject";
}

void MtpServer::commitEdit(ObjectEdit* edit) {
    mDatabase->endSendObject(edit->mPath.c_str(), edit->mHandle, edit->mFormat, true);
}


bool MtpServer::handleRequest() {
    MtpAutolock autoLock(mMutex);

    MtpOperationCode operation = mRequest.getOperationCode();
    MtpResponseCode response;

    mResponse.reset();

    if (mSendObjectHandle != kInvalidObjectHandle && operation != MTP_OPERATION_SEND_OBJECT) {
        // FIXME - need to delete mSendObjectHandle from the database
        LOG(ERROR) << "expected SendObject after SendObjectInfo";
        mSendObjectHandle = kInvalidObjectHandle;
    }

    switch (operation) {
        case MTP_OPERATION_GET_DEVICE_INFO:
            response = doGetDeviceInfo();
            break;
        case MTP_OPERATION_OPEN_SESSION:
            response = doOpenSession();
            break;
        case MTP_OPERATION_CLOSE_SESSION:
            response = doCloseSession();
            break;
        case MTP_OPERATION_GET_STORAGE_IDS:
            response = doGetStorageIDs();
            break;
         case MTP_OPERATION_GET_STORAGE_INFO:
            response = doGetStorageInfo();
            break;
        case MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED:
            response = doGetObjectPropsSupported();
            break;
        case MTP_OPERATION_GET_OBJECT_HANDLES:
            response = doGetObjectHandles();
            break;
        case MTP_OPERATION_GET_NUM_OBJECTS:
            response = doGetNumObjects();
            break;
        case MTP_OPERATION_GET_OBJECT_REFERENCES:
            response = doGetObjectReferences();
            break;
        case MTP_OPERATION_SET_OBJECT_REFERENCES:
            response = doSetObjectReferences();
            break;
        case MTP_OPERATION_GET_OBJECT_PROP_VALUE:
            response = doGetObjectPropValue();
            break;
        case MTP_OPERATION_SET_OBJECT_PROP_VALUE:
            response = doSetObjectPropValue();
            break;
        case MTP_OPERATION_GET_DEVICE_PROP_VALUE:
            response = doGetDevicePropValue();
            break;
        case MTP_OPERATION_SET_DEVICE_PROP_VALUE:
            response = doSetDevicePropValue();
            break;
        case MTP_OPERATION_RESET_DEVICE_PROP_VALUE:
            response = doResetDevicePropValue();
            break;
        case MTP_OPERATION_GET_OBJECT_PROP_LIST:
            response = doGetObjectPropList();
            break;
        case MTP_OPERATION_GET_OBJECT_INFO:
            response = doGetObjectInfo();
            break;
        case MTP_OPERATION_GET_OBJECT:
            response = doGetObject();
            break;
        case MTP_OPERATION_GET_THUMB:
            response = doGetThumb();
            break;
        case MTP_OPERATION_GET_PARTIAL_OBJECT:
        case MTP_OPERATION_GET_PARTIAL_OBJECT_64:
            response = doGetPartialObject(operation);
            break;
        case MTP_OPERATION_SEND_OBJECT_INFO:
            response = doSendObjectInfo();
            break;
        case MTP_OPERATION_SEND_OBJECT:
            response = doSendObject();
            break;
        case MTP_OPERATION_DELETE_OBJECT:
            response = doDeleteObject();
            break;
        case MTP_OPERATION_MOVE_OBJECT:
            response = doMoveObject();
            break;
        case MTP_OPERATION_GET_OBJECT_PROP_DESC:
            response = doGetObjectPropDesc();
            break;
        case MTP_OPERATION_GET_DEVICE_PROP_DESC:
            response = doGetDevicePropDesc();
            break;
        case MTP_OPERATION_SEND_PARTIAL_OBJECT:
            response = doSendPartialObject();
            break;
        case MTP_OPERATION_TRUNCATE_OBJECT:
            response = doTruncateObject();
            break;
        case MTP_OPERATION_BEGIN_EDIT_OBJECT:
            response = doBeginEditObject();
            break;
        case MTP_OPERATION_END_EDIT_OBJECT:
            response = doEndEditObject();
            break;
        default:
            LOG(ERROR) << "got unsupported command " << MtpDebug::getOperationCodeName(operation);
            response = MTP_RESPONSE_OPERATION_NOT_SUPPORTED;
            break;
    }

    mResponse.setResponseCode(response);
    return true;
}

MtpResponseCode MtpServer::doGetDeviceInfo() {
    VLOG(1) <<  __PRETTY_FUNCTION__;
    MtpStringBuffer   string;
    //char prop_value[PROP_VALUE_MAX];

    MtpObjectFormatList* playbackFormats = mDatabase->getSupportedPlaybackFormats();
    MtpObjectFormatList* captureFormats = mDatabase->getSupportedCaptureFormats();
    MtpDevicePropertyList* deviceProperties = mDatabase->getSupportedDeviceProperties();

    // fill in device info
    mData.putUInt16(MTP_STANDARD_VERSION);
    if (mPtp) {
        mData.putUInt32(0);
    } else {
        // MTP Vendor Extension ID
        mData.putUInt32(6);
    }
    mData.putUInt16(MTP_STANDARD_VERSION);
    if (mPtp) {
        // no extensions
        string.set("");
    } else {
        // MTP extensions
        string.set("microsoft.com: 1.0; android.com: 1.0;");
    }
    mData.putString(string); // MTP Extensions
    mData.putUInt16(0); //Functional Mode
    mData.putAUInt16(kSupportedOperationCodes,
            sizeof(kSupportedOperationCodes) / sizeof(uint16_t)); // Operations Supported
    mData.putAUInt16(kSupportedEventCodes,
            sizeof(kSupportedEventCodes) / sizeof(uint16_t)); // Events Supported
    mData.putAUInt16(deviceProperties); // Device Properties Supported
    mData.putAUInt16(captureFormats); // Capture Formats
    mData.putAUInt16(playbackFormats);  // Playback Formats

    //property_get("ro.product.manufacturer", prop_value, "unknown manufacturer");
    string.set("Simple Mod Manager");
    mData.putString(string);   // Manufacturer

    //property_get("ro.product.model", prop_value, "MTP Device");
    string.set("Simple Mod Manager MTP");
    mData.putString(string);   // Model
    string.set("1.0");
    mData.putString(string);   // Device Version

    //property_get("ro.serialno", prop_value, "????????");
    string.set("SMMTP001");
    mData.putString(string);   // Serial Number

    delete playbackFormats;
    delete captureFormats;
    delete deviceProperties;

    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doOpenSession() {
    if (mSessionOpen) {
        mResponse.setParameter(1, mSessionID);
        return MTP_RESPONSE_SESSION_ALREADY_OPEN;
    }
    mSessionID = mRequest.getParameter(1);
    mSessionOpen = true;

    mDatabase->sessionStarted(this);

    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doCloseSession() {
    if (!mSessionOpen)
        return MTP_RESPONSE_SESSION_NOT_OPEN;
    mSessionID = 0;
    mSessionOpen = false;
    mDatabase->sessionEnded();
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doGetStorageIDs() {
    if (!mSessionOpen)
        return MTP_RESPONSE_SESSION_NOT_OPEN;

    int count = mStorages.size();
    mData.putUInt32(count);
    for (int i = 0; i < count; i++)
        mData.putUInt32(mStorages[i]->getStorageID());

    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doGetStorageInfo() {
    MtpStringBuffer   string;

    if (!mSessionOpen)
        return MTP_RESPONSE_SESSION_NOT_OPEN;
    MtpStorageID id = mRequest.getParameter(1);
    MtpStorage* storage = getStorage(id);
    if (!storage)
        return MTP_RESPONSE_INVALID_STORAGE_ID;

    mData.putUInt16(storage->getType());
    mData.putUInt16(storage->getFileSystemType());
    mData.putUInt16(storage->getAccessCapability());
    mData.putUInt64(storage->getMaxCapacity());
    mData.putUInt64(storage->getFreeSpace());
    mData.putUInt32(1024*1024*1024); // Free Space in Objects
    string.set(storage->getDescription());
    mData.putString(string);
    mData.putEmptyString();   // Volume Identifier

    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doGetObjectPropsSupported() {
    if (!mSessionOpen)
        return MTP_RESPONSE_SESSION_NOT_OPEN;
    MtpObjectFormat format = mRequest.getParameter(1);
    MtpObjectPropertyList* properties = mDatabase->getSupportedObjectProperties(format);
    mData.putAUInt16(properties);
    delete properties;
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doGetObjectHandles() {
    if (!mSessionOpen)
        return MTP_RESPONSE_SESSION_NOT_OPEN;
    MtpStorageID storageID = mRequest.getParameter(1);      // 0xFFFFFFFF for all storage
    MtpObjectFormat format = mRequest.getParameter(2);      // 0 for all formats
    MtpObjectHandle parent = mRequest.getParameter(3);      // 0xFFFFFFFF for objects with no parent
                                                            // 0x00000000 for all objects

    if (!hasStorage(storageID))
        return MTP_RESPONSE_INVALID_STORAGE_ID;

    MtpObjectHandleList* handles = mDatabase->getObjectList(storageID, format, parent);
    mData.putAUInt32(handles);
    delete handles;
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doGetNumObjects() {
    if (!mSessionOpen)
        return MTP_RESPONSE_SESSION_NOT_OPEN;
    MtpStorageID storageID = mRequest.getParameter(1);      // 0xFFFFFFFF for all storage
    MtpObjectFormat format = mRequest.getParameter(2);      // 0 for all formats
    MtpObjectHandle parent = mRequest.getParameter(3);      // 0xFFFFFFFF for objects with no parent
                                                            // 0x00000000 for all objects
    if (!hasStorage(storageID))
        return MTP_RESPONSE_INVALID_STORAGE_ID;

    int count = mDatabase->getNumObjects(storageID, format, parent);
    if (count >= 0) {
        mResponse.setParameter(1, count);
        return MTP_RESPONSE_OK;
    } else {
        mResponse.setParameter(1, 0);
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    }
}

MtpResponseCode MtpServer::doGetObjectReferences() {
    if (!mSessionOpen)
        return MTP_RESPONSE_SESSION_NOT_OPEN;
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpObjectHandle handle = mRequest.getParameter(1);

    if (!mDatabase->isHandleValid(handle)) {
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    }

    MtpObjectHandleList* handles = mDatabase->getObjectReferences(handle);
    if (handles) {
        mData.putAUInt32(handles);
        delete handles;
    } else {
        mData.putEmptyArray();
    }
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doSetObjectReferences() {
    if (!mSessionOpen)
        return MTP_RESPONSE_SESSION_NOT_OPEN;
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpStorageID handle = mRequest.getParameter(1);

    MtpObjectHandleList* references = mData.getAUInt32();
    MtpResponseCode result = mDatabase->setObjectReferences(handle, references);
    delete references;
    return result;
}

MtpResponseCode MtpServer::doGetObjectPropValue() {
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpObjectHandle handle = mRequest.getParameter(1);
    MtpObjectProperty property = mRequest.getParameter(2);
    VLOG(2) << "GetObjectPropValue " << handle
            << " " << MtpDebug::getObjectPropCodeName(property);

    return mDatabase->getObjectPropertyValue(handle, property, mData);
}

MtpResponseCode MtpServer::doSetObjectPropValue() {
    MtpResponseCode response;

    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpObjectHandle handle = mRequest.getParameter(1);
    MtpObjectProperty property = mRequest.getParameter(2);
    VLOG(2) << "SetObjectPropValue " << handle
            << " " << MtpDebug::getObjectPropCodeName(property);

    response = mDatabase->setObjectPropertyValue(handle, property, mData);

    //sendObjectPropChanged(handle, property);

    return response;
}

MtpResponseCode MtpServer::doGetDevicePropValue() {
    MtpDeviceProperty property = mRequest.getParameter(1);
    VLOG(1) << "GetDevicePropValue " << MtpDebug::getDevicePropCodeName(property);

    return mDatabase->getDevicePropertyValue(property, mData);
}

MtpResponseCode MtpServer::doSetDevicePropValue() {
    MtpDeviceProperty property = mRequest.getParameter(1);
    VLOG(1) << "SetDevicePropValue " << MtpDebug::getDevicePropCodeName(property);

    return mDatabase->setDevicePropertyValue(property, mData);
}

MtpResponseCode MtpServer::doResetDevicePropValue() {
    MtpDeviceProperty property = mRequest.getParameter(1);
    VLOG(1) << "ResetDevicePropValue " << MtpDebug::getDevicePropCodeName(property);

    return mDatabase->resetDeviceProperty(property);
}

MtpResponseCode MtpServer::doGetObjectPropList() {
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

    MtpObjectHandle handle = mRequest.getParameter(1);
    // use uint32_t so we can support 0xFFFFFFFF
    uint32_t format = mRequest.getParameter(2);
    uint32_t property = mRequest.getParameter(3);
    int groupCode = mRequest.getParameter(4);
    int depth = mRequest.getParameter(5);
    VLOG(2) << "GetObjectPropList " << handle
            << " format: " << MtpDebug::getFormatCodeName(format)
            << " property: " << MtpDebug::getObjectPropCodeName(property)
            << " group: " << groupCode
            << " depth: " << depth;

    return mDatabase->getObjectPropertyList(handle, format, property, groupCode, depth, mData);
}

MtpResponseCode MtpServer::doGetObjectInfo() {
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpObjectHandle handle = mRequest.getParameter(1);
    MtpObjectInfo info(handle);
    MtpResponseCode result = mDatabase->getObjectInfo(handle, info);
    if (result == MTP_RESPONSE_OK) {
        char    date[20];

        mData.putUInt32(info.mStorageID);
        mData.putUInt16(info.mFormat);
        mData.putUInt16(info.mProtectionStatus);

        // if object is being edited the database size may be out of date
        uint32_t size = info.mCompressedSize;
        ObjectEdit* edit = getEditObject(handle);
        if (edit)
            size = (edit->mSize > 0xFFFFFFFFLL ? 0xFFFFFFFF : (uint32_t)edit->mSize);
        mData.putUInt32(size);

        mData.putUInt16(info.mThumbFormat);
        mData.putUInt32(info.mThumbCompressedSize);
        mData.putUInt32(info.mThumbPixWidth);
        mData.putUInt32(info.mThumbPixHeight);
        mData.putUInt32(info.mImagePixWidth);
        mData.putUInt32(info.mImagePixHeight);
        mData.putUInt32(info.mImagePixDepth);
        mData.putUInt32(info.mParent);
        mData.putUInt16(info.mAssociationType);
        mData.putUInt32(info.mAssociationDesc);
        mData.putUInt32(info.mSequenceNumber);
        mData.putString(info.mName);
        mData.putEmptyString();    // date created
        formatDateTime(info.mDateModified, date, sizeof(date));
        mData.putString(date);   // date modified
        mData.putEmptyString();   // keywords
    }
    return result;
}

struct mtp_file_range {
    int fd;
    off_t offset;
    int64_t length;
    uint16_t command;
    uint32_t transaction_id;
};

static int send_file(USBMtpInterface* usb, struct mtp_file_range * mfr)
{
    int actualsize;
    int j, ofs;
    int blocksize;

    struct stat buf;
    fstat(mfr->fd, &buf);

    unsigned char * buffer = getReusableTransferBuffer(kMtpTransferChunkSize + MTP_CONTAINER_HEADER_SIZE);
    if (buffer == nullptr)
        return -1;
    *(uint32_t*)&buffer[0] = mfr->length + MTP_CONTAINER_HEADER_SIZE;
    *(uint16_t*)&buffer[4] = MTP_CONTAINER_TYPE_DATA;
    *(uint16_t*)&buffer[6] = mfr->command;
    *(uint32_t*)&buffer[8] = mfr->transaction_id;

    if(mfr->offset >= buf.st_size)
    {
        actualsize = 0;
    }
    else
    {
      if(mfr->offset + mfr->length > buf.st_size)
          actualsize = buf.st_size - mfr->offset;
      else
          actualsize = mfr->length;
    }

    lseek(mfr->fd, mfr->offset, SEEK_SET);
    ofs = MTP_CONTAINER_HEADER_SIZE;
    j = 0;
    do
    {
        if((j + (kMtpTransferChunkSize - ofs)) < actualsize)
            blocksize = (kMtpTransferChunkSize - ofs);
        else
            blocksize = actualsize - j;

        read(mfr->fd, &buffer[ofs], blocksize);
        j += blocksize;
        ofs += blocksize;

        usb->write((const char*)buffer, ofs);
        ofs = 0;
    } while(j < actualsize);

    return actualsize;
}

static bool writeFully(int fd, const uint8_t* buffer, size_t size)
{
    size_t total = 0;

    while (total < size) {
        ssize_t written = write(fd, buffer + total, size - total);
        if (written < 0) {
            return false;
        }
        if (written == 0) {
            errno = EIO;
            return false;
        }
        total += static_cast<size_t>(written);
    }

    return true;
}

static uint32_t getInitialObjectReadSize(uint64_t payloadSize)
{
    uint64_t desired = kMtpInitialObjectReadSize;
    if (payloadSize != 0xFFFFFFFFULL) {
        desired = payloadSize + MTP_CONTAINER_HEADER_SIZE;
        if (desired < MTP_CONTAINER_HEADER_SIZE)
            desired = MTP_CONTAINER_HEADER_SIZE;
        if (desired > kMtpInitialObjectReadSize)
            desired = kMtpInitialObjectReadSize;
    }
    return static_cast<uint32_t>(desired);
}

static int64_t receive_file(USBMtpInterface* usb, struct mtp_file_range * mfr)
{
    if(mfr->length < 0 || mfr->length == 0xFFFFFFFF) {
        errno = EINVAL;
        return -1;
    }

    unsigned char * buffer = getReusableTransferBuffer(kMtpTransferChunkSize);
    if (buffer == nullptr)
        return -1;

    if (lseek(mfr->fd, mfr->offset, SEEK_SET) < 0)
        return -1;

    int64_t total = 0;
    while(total < mfr->length)
    {
        const size_t remaining = static_cast<size_t>(
                std::min<int64_t>(static_cast<int64_t>(kMtpTransferChunkSize),
                                  mfr->length - total));
        ssize_t size = usb->readWithTimeout((char*)buffer, remaining, kMtpDataPhaseTimeoutNs);
        if (size < 0) {
            if (errno == 0)
                errno = EIO;
            return -1;
        }
        if (size == 0) {
            errno = EIO;
            return -1;
        }

        if (!writeFully(mfr->fd, buffer, static_cast<size_t>(size)))
            return -1;

        total += size;
    }

    return total;
}

MtpResponseCode MtpServer::doGetObject() {
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpObjectHandle handle = mRequest.getParameter(1);
    MtpString pathBuf;
    int64_t fileLength;
    MtpObjectFormat format;
    int result = mDatabase->getObjectFilePath(handle, pathBuf, fileLength, format);
    if (result != MTP_RESPONSE_OK)
        return result;
    if (isBlockedReadPath(pathBuf.c_str()))
        return MTP_RESPONSE_ACCESS_DENIED;

    struct mtp_file_range mfr;
    mfr.fd = open(pathBuf.c_str(), O_RDONLY);
    if (mfr.fd < 0) {
        return MTP_RESPONSE_GENERAL_ERROR;
    }
    mfr.offset = 0;
    mfr.length = fileLength;
    mfr.command = mRequest.getOperationCode();
    mfr.transaction_id = mRequest.getTransactionID();

    // then transfer the file
    int ret = send_file(mUSB, &mfr);
    VLOG(2) << "MTP_SEND_FILE_WITH_HEADER returned " << ret;
    close(mfr.fd);
    if (ret < 0) {
        if (errno == ECANCELED)
            return MTP_RESPONSE_TRANSACTION_CANCELLED;
        else
            return MTP_RESPONSE_GENERAL_ERROR;
    }
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doGetThumb() {
    MtpObjectHandle handle = mRequest.getParameter(1);
    size_t thumbSize;
    void* thumb = mDatabase->getThumbnail(handle, thumbSize);
    if (thumb) {
        // send data
        mData.setOperationCode(mRequest.getOperationCode());
        mData.setTransactionID(mRequest.getTransactionID());
        mData.writeData(mUSB, thumb, thumbSize);
        free(thumb);
        return MTP_RESPONSE_OK;
    } else {
        return MTP_RESPONSE_GENERAL_ERROR;
    }
}

MtpResponseCode MtpServer::doGetPartialObject(MtpOperationCode operation) {
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpObjectHandle handle = mRequest.getParameter(1);
    uint64_t offset;
    uint32_t length;
    offset = mRequest.getParameter(2);
    if (operation == MTP_OPERATION_GET_PARTIAL_OBJECT_64) {
        // android extension with 64 bit offset
        uint64_t offset2 = mRequest.getParameter(3);
        offset = offset | (offset2 << 32);
        length = mRequest.getParameter(4);
    } else {
        // standard GetPartialObject
        length = mRequest.getParameter(3);
    }
    MtpString pathBuf;
    int64_t fileLength;
    MtpObjectFormat format;
    int result = mDatabase->getObjectFilePath(handle, pathBuf, fileLength, format);
    if (result != MTP_RESPONSE_OK)
        return result;
    if (isBlockedReadPath(pathBuf.c_str()))
        return MTP_RESPONSE_ACCESS_DENIED;
    if (offset + length > fileLength)
        length = fileLength - offset;

    mtp_file_range  mfr;
    mfr.fd = open(pathBuf.c_str(), O_RDONLY);
    if (mfr.fd < 0) {
        return MTP_RESPONSE_GENERAL_ERROR;
    }
    mfr.offset = offset;
    mfr.length = length;
    mfr.command = mRequest.getOperationCode();
    mfr.transaction_id = mRequest.getTransactionID();
    mResponse.setParameter(1, length);

    // transfer the file
    int ret = send_file(mUSB, &mfr);
    VLOG(2) << "MTP_SEND_FILE_WITH_HEADER returned " << ret;
    close(mfr.fd);
    if (ret < 0) {
        if (errno == ECANCELED)
            return MTP_RESPONSE_TRANSACTION_CANCELLED;
        else
            return MTP_RESPONSE_GENERAL_ERROR;
    }
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doSendObjectInfo() {
    MtpString path;
    MtpStorageID storageID = mRequest.getParameter(1);
    MtpStorage* storage = getStorage(storageID);
    MtpObjectHandle parent = mRequest.getParameter(2);
    if (!storage)
        return MTP_RESPONSE_INVALID_STORAGE_ID;

    // special case the root
    if (parent == MTP_PARENT_ROOT) {
        path = storage->getPath();
        parent = 0;
    } else {
        int64_t length;
        MtpObjectFormat format;
        int result = mDatabase->getObjectFilePath(parent, path, length, format);
        if (result != MTP_RESPONSE_OK)
            return result;
        if (format != MTP_FORMAT_ASSOCIATION)
            return MTP_RESPONSE_INVALID_PARENT_OBJECT;
    }

    // read only the fields we need
    mData.getUInt32();  // storage ID
    MtpObjectFormat format = mData.getUInt16();
    mData.getUInt16();  // protection status
    mSendObjectFileSize = mData.getUInt32();
    mData.getUInt16();  // thumb format
    mData.getUInt32();  // thumb compressed size
    mData.getUInt32();  // thumb pix width
    mData.getUInt32();  // thumb pix height
    mData.getUInt32();  // image pix width
    mData.getUInt32();  // image pix height
    mData.getUInt32();  // image bit depth
    mData.getUInt32();  // parent
    uint16_t associationType = mData.getUInt16();
    uint32_t associationDesc = mData.getUInt32();   // association desc
    mData.getUInt32();  // sequence number
    MtpStringBuffer name, created, modified;
    mData.getString(name);    // file name
    mData.getString(created);      // date created
    mData.getString(modified);     // date modified
    // keywords follow

    VLOG(2) << "name: " << (const char *) name
            << " format: " << std::hex << format << std::dec;
    time_t modifiedTime;
    if (!parseDateTime(modified, modifiedTime))
        modifiedTime = 0;

    if (path[path.size() - 1] != '/')
        path += "/";
    path += (const char *)name;

    // check space first
    if (mSendObjectFileSize > storage->getFreeSpace())
        return MTP_RESPONSE_STORAGE_FULL;
    uint64_t maxFileSize = storage->getMaxFileSize();
    // check storage max file size
    if (maxFileSize != 0) {
        // if mSendObjectFileSize is 0xFFFFFFFF, then all we know is the file size
        // is >= 0xFFFFFFFF
        if (mSendObjectFileSize > maxFileSize || mSendObjectFileSize == 0xFFFFFFFF)
            return MTP_RESPONSE_OBJECT_TOO_LARGE;
    }

    VLOG(2) << "path: " << path.c_str() << " parent: " << parent
            << " storageID: " << std::hex << storageID << std::dec;
    MtpObjectHandle handle = mDatabase->beginSendObject(path.c_str(),
            format, parent, storageID, mSendObjectFileSize, modifiedTime);
    if (handle == kInvalidObjectHandle) {
        return MTP_RESPONSE_GENERAL_ERROR;
    }

  if (format == MTP_FORMAT_ASSOCIATION) {
        int ret = mkdir(path.c_str(), mDirectoryPermission);
        if (ret && errno != EEXIST) {
            mDatabase->endSendObject(path, handle, MTP_FORMAT_ASSOCIATION, false);
            return MTP_RESPONSE_GENERAL_ERROR;
        }
        if (ret && errno == EEXIST) {
            struct stat st;
            if (stat(path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
                mDatabase->endSendObject(path, handle, MTP_FORMAT_ASSOCIATION, false);
                return MTP_RESPONSE_GENERAL_ERROR;
            }
        }

        // SendObject does not get sent for directories, so call endSendObject here instead
        mDatabase->endSendObject(path, handle, MTP_FORMAT_ASSOCIATION, MTP_RESPONSE_OK);
    } else {
        mSendObjectFilePath = path;
        // save the handle for the SendObject call, which should follow
        mSendObjectHandle = handle;
        mSendObjectFormat = format;
    }

    mResponse.setParameter(1, storageID);
    mResponse.setParameter(2, parent);
    mResponse.setParameter(3, handle);

    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doSendObject() {
    if (!hasStorage())
        return MTP_RESPONSE_GENERAL_ERROR;
    MtpResponseCode result = MTP_RESPONSE_OK;
    mtp_file_range  mfr;
    mfr.fd = -1;
    int ret = 0;
    int initialData = 0;
    size_t initialPayload = 0;
    uint64_t expectedPayload = mSendObjectFileSize;
    uint32_t containerLength = 0;

    if (mSendObjectHandle == kInvalidObjectHandle) {
        LOG(ERROR) << "Expected SendObjectInfo before SendObject";
        mData.reset();
        return MTP_RESPONSE_NO_VALID_OBJECT_INFO;
    }

    // read the header, and possibly some data
    ret = mData.readWithTimeout(mUSB, getInitialObjectReadSize(mSendObjectFileSize), kMtpDataPhaseTimeoutNs);
    if (ret < MTP_CONTAINER_HEADER_SIZE) {
        result = MTP_RESPONSE_GENERAL_ERROR;
        goto done;
    }
    initialData = ret - MTP_CONTAINER_HEADER_SIZE;
    initialPayload = static_cast<size_t>(initialData);

    containerLength = mData.getContainerLength();
    if (containerLength != 0xFFFFFFFFU) {
        if (containerLength < MTP_CONTAINER_HEADER_SIZE) {
            result = MTP_RESPONSE_GENERAL_ERROR;
            goto done;
        }
        expectedPayload = static_cast<uint64_t>(containerLength - MTP_CONTAINER_HEADER_SIZE);
    } else if (expectedPayload == 0xFFFFFFFFULL) {
        result = MTP_RESPONSE_OBJECT_TOO_LARGE;
        goto done;
    }

    if (initialPayload > expectedPayload) {
        result = MTP_RESPONSE_GENERAL_ERROR;
        goto done;
    }

    mfr.fd = open(mSendObjectFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (mfr.fd < 0) {
        result = MTP_RESPONSE_GENERAL_ERROR;
        goto done;
    }

    if (initialData > 0 && !writeFully(mfr.fd, mData.getData(), initialPayload)) {
        result = (errno == ECANCELED)
                ? MTP_RESPONSE_TRANSACTION_CANCELLED
                : MTP_RESPONSE_GENERAL_ERROR;
        goto done;
    }

    if (expectedPayload > initialPayload) {
        const uint64_t remainingPayload = expectedPayload - initialPayload;
        if (remainingPayload > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            result = MTP_RESPONSE_OBJECT_TOO_LARGE;
            goto done;
        }

        mfr.offset = initialPayload;
        mfr.length = static_cast<int64_t>(remainingPayload);

        VLOG(2) << "receiving " << mSendObjectFilePath.c_str();
        // transfer the file
        int64_t received = receive_file(mUSB, &mfr);
        VLOG(2) << "MTP_RECEIVE_FILE returned " << received;
        if (received < 0 || received != mfr.length) {
            result = MTP_RESPONSE_TRANSACTION_CANCELLED;
            if (errno != ECANCELED)
                result = MTP_RESPONSE_GENERAL_ERROR;
            goto done;
        }
    }

done:
    if (result == MTP_RESPONSE_OK)
        mDatabase->updateObjectSize(mSendObjectHandle, expectedPayload);
    if (mfr.fd >= 0)
        close(mfr.fd);
    if (result != MTP_RESPONSE_OK)
        unlink(mSendObjectFilePath.c_str());

    // reset so we don't attempt to send the data back
    mData.reset();

    mDatabase->endSendObject(mSendObjectFilePath, mSendObjectHandle, mSendObjectFormat,
            result == MTP_RESPONSE_OK);
    mSendObjectHandle = kInvalidObjectHandle;
    mSendObjectFormat = 0;
    return result;
}

static void deleteRecursive(const char* path) {
    char pathbuf[PATH_MAX];
    int pathLength = strlen(path);
    if (pathLength >= sizeof(pathbuf) - 1) {
        LOG(ERROR) << "path too long: " << path;
    }
    strcpy(pathbuf, path);
    if (pathbuf[pathLength - 1] != '/') {
        pathbuf[pathLength++] = '/';
    }
    char* fileSpot = pathbuf + pathLength;
    int pathRemaining = sizeof(pathbuf) - pathLength - 1;

    DIR* dir = opendir(path);
    if (!dir) {
        LOG(ERROR) << "opendir " << path << " failed";
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        const char* name = entry->d_name;

        // ignore "." and ".."
        if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) {
            continue;
        }

        int nameLength = strlen(name);
        if (nameLength > pathRemaining) {
            LOG(ERROR) << "path " << path << "/" << name << " too long";
            continue;
        }
        strcpy(fileSpot, name);

        int type = entry->d_type;
        if (entry->d_type == DT_DIR) {
            deleteRecursive(pathbuf);
            rmdir(pathbuf);
        } else {
            unlink(pathbuf);
        }
    }
    closedir(dir);
}

static void deletePath(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) == 0) {
        if (S_ISDIR(statbuf.st_mode)) {
            deleteRecursive(path);
            rmdir(path);
        } else {
            unlink(path);
        }
    } else {
        LOG(ERROR) << "deletePath stat failed for " << path;
    }
}

MtpResponseCode MtpServer::doDeleteObject() {
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpObjectHandle handle = mRequest.getParameter(1);
    MtpObjectFormat format = mRequest.getParameter(2);
    // FIXME - support deleting all objects if handle is 0xFFFFFFFF
    // FIXME - implement deleting objects by format

    MtpString filePath;
    int64_t fileLength;
    int result = mDatabase->getObjectFilePath(handle, filePath, fileLength, format);
    if (result == MTP_RESPONSE_OK) {
        VLOG(2) << "deleting " << filePath.c_str();
        result = mDatabase->deleteFile(handle);
        // Don't delete the actual files unless the database deletion is allowed
        if (result == MTP_RESPONSE_OK) {
            deletePath(filePath.c_str());
        }
    }

    return result;
}

MtpResponseCode MtpServer::doMoveObject() {
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpObjectHandle handle = mRequest.getParameter(1);
    MtpObjectFormat format = mRequest.getParameter(2);
    MtpObjectHandle newparent = mRequest.getParameter(3);

    MtpString filePath;
    MtpString newPath;
    int64_t fileLength;
    int result = mDatabase->getObjectFilePath(handle, filePath, fileLength, format);
    result = mDatabase->getObjectFilePath(handle, newPath, fileLength, format);
    if (result == MTP_RESPONSE_OK) {
        VLOG(2) << "moving " << filePath.c_str() << " to " << newPath.c_str();
        result = mDatabase->moveFile(handle, newparent);
        // Don't move the actual files unless the database deletion is allowed
        if (result == MTP_RESPONSE_OK) {
            rename(filePath.c_str(), newPath.c_str());
        }
    }

    return result;
}

MtpResponseCode MtpServer::doGetObjectPropDesc() {
    MtpObjectProperty propCode = mRequest.getParameter(1);
    MtpObjectFormat format = mRequest.getParameter(2);
    VLOG(2) << "GetObjectPropDesc " << MtpDebug::getObjectPropCodeName(propCode)
            << " " << MtpDebug::getFormatCodeName(format);
    MtpProperty* property = mDatabase->getObjectPropertyDesc(propCode, format);
    if (!property)
        return MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED;
    property->write(mData);
    delete property;
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doGetDevicePropDesc() {
    MtpDeviceProperty propCode = mRequest.getParameter(1);
    VLOG(1) << "GetDevicePropDesc " << MtpDebug::getDevicePropCodeName(propCode);
    MtpProperty* property = mDatabase->getDevicePropertyDesc(propCode);
    if (!property)
        return MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
    property->write(mData);
    delete property;
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doSendPartialObject() {
    if (!hasStorage())
        return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
    MtpObjectHandle handle = mRequest.getParameter(1);
    uint64_t offset = mRequest.getParameter(2);
    uint64_t offset2 = mRequest.getParameter(3);
    offset = offset | (offset2 << 32);
    uint32_t length = mRequest.getParameter(4);
    const uint64_t originalOffset = offset;
    const uint32_t originalLength = length;

    ObjectEdit* edit = getEditObject(handle);
    if (!edit) {
        LOG(ERROR) << "object not open for edit in doSendPartialObject";
        return MTP_RESPONSE_GENERAL_ERROR;
    }

    // can't start writing past the end of the file
    if (offset > edit->mSize) {
        VLOG(2) << "writing past end of object, offset: " << offset
                << " edit->mSize: " << edit->mSize;
        return MTP_RESPONSE_GENERAL_ERROR;
    }

    const char* filePath = edit->mPath.c_str();
    VLOG(2) << "receiving partial " << filePath
            << " " << offset << " " << length;

    // read the header, and possibly some data
    int ret = mData.readWithTimeout(mUSB, getInitialObjectReadSize(length), kMtpDataPhaseTimeoutNs);
    if (ret < MTP_CONTAINER_HEADER_SIZE)
        return MTP_RESPONSE_GENERAL_ERROR;
    int initialData = ret - MTP_CONTAINER_HEADER_SIZE;
    if (static_cast<uint32_t>(initialData) > length)
        return MTP_RESPONSE_GENERAL_ERROR;

    if (initialData > 0) {
        if (!writeFully(edit->mFD, mData.getData(), static_cast<size_t>(initialData)))
            ret = -1;
        offset += initialData;
        length -= initialData;
    }

    if (ret >= 0 && length > 0) {
        mtp_file_range  mfr;
        mfr.fd = edit->mFD;
        mfr.offset = offset;
        mfr.length = length;

        // transfer the file
        int64_t received = receive_file(mUSB, &mfr);
        VLOG(2) << "MTP_RECEIVE_FILE returned " << received;
        ret = (received == mfr.length) ? 0 : -1;
    }
    if (ret < 0) {
        mResponse.setParameter(1, 0);
        if (errno == ECANCELED)
            return MTP_RESPONSE_TRANSACTION_CANCELLED;
        else
            return MTP_RESPONSE_GENERAL_ERROR;
    }

    // reset so we don't attempt to send this back
    mData.reset();
    mResponse.setParameter(1, originalLength);
    uint64_t end = originalOffset + originalLength;
    if (end > edit->mSize) {
        edit->mSize = end;
    }
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doTruncateObject() {
    MtpObjectHandle handle = mRequest.getParameter(1);
    ObjectEdit* edit = getEditObject(handle);
    if (!edit) {
        LOG(ERROR) << "object not open for edit in doTruncateObject";
        return MTP_RESPONSE_GENERAL_ERROR;
    }

    uint64_t offset = mRequest.getParameter(2);
    uint64_t offset2 = mRequest.getParameter(3);
    offset |= (offset2 << 32);
    if (ftruncate(edit->mFD, offset) != 0) {
        return MTP_RESPONSE_GENERAL_ERROR;
    } else {
        edit->mSize = offset;
        return MTP_RESPONSE_OK;
    }
}

MtpResponseCode MtpServer::doBeginEditObject() {
    MtpObjectHandle handle = mRequest.getParameter(1);
    if (getEditObject(handle)) {
        LOG(ERROR) << "object already open for edit in doBeginEditObject";
        return MTP_RESPONSE_GENERAL_ERROR;
    }

    MtpString path;
    int64_t fileLength;
    MtpObjectFormat format;
    int result = mDatabase->getObjectFilePath(handle, path, fileLength, format);
    if (result != MTP_RESPONSE_OK)
        return result;

    int fd = open(path.c_str(), O_RDWR | O_EXCL);
    if (fd < 0) {
        LOG(ERROR) << "open failed for " << path.c_str() << " in doBeginEditObject";
        return MTP_RESPONSE_GENERAL_ERROR;
    }

    addEditObject(handle, path, fileLength, format, fd);
    return MTP_RESPONSE_OK;
}

MtpResponseCode MtpServer::doEndEditObject() {
    MtpObjectHandle handle = mRequest.getParameter(1);
    ObjectEdit* edit = getEditObject(handle);
    if (!edit) {
        LOG(ERROR) << "object not open for edit in doEndEditObject";
        return MTP_RESPONSE_GENERAL_ERROR;
    }

    commitEdit(edit);
    removeEditObject(handle);
    return MTP_RESPONSE_OK;
}

}  // namespace android
