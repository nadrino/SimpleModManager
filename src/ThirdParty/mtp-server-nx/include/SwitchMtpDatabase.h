/*
 * Copyright (C) 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef STUB_MTP_DATABASE_H_
#define STUB_MTP_DATABASE_H_

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <exception>
#include <algorithm>
#include <filesystem>
#include <unordered_map>
#include <sys/stat.h>

#include "mtp.h"
#include "MtpDatabase.h"
#include "MtpDataPacket.h"
#include "MtpStringBuffer.h"
#include "MtpObjectInfo.h"
#include "MtpProperty.h"
#include "MtpDebug.h"

#include "log.h"

#define ALL_PROPERTIES 0xffffffff

using namespace std::filesystem;

namespace android
{
class SwitchMtpDatabase : public android::MtpDatabase {
private:
    struct DbEntry
    {
        MtpStorageID storage_id;
        MtpObjectFormat object_format;
        MtpObjectHandle parent;
        size_t object_size;
        std::string display_name;
        std::string path;
        std::time_t last_modified;
        bool scanned = false;
    };

    MtpServer* local_server;
    bool show_debug_mtp_files;
    uint32_t counter;
    std::map<MtpObjectHandle, DbEntry> db;
    std::unordered_map<MtpObjectHandle, std::vector<MtpObjectHandle>> children_by_parent;
    std::map<std::string, MtpObjectFormat> formats = {
        {".gif", MTP_FORMAT_GIF},
        {".png", MTP_FORMAT_PNG},
        {".jpeg", MTP_FORMAT_JFIF},
        {".tiff", MTP_FORMAT_TIFF},
        {".ogg", MTP_FORMAT_OGG},
        {".mp3", MTP_FORMAT_MP3},
        {".wav", MTP_FORMAT_WAV},
        {".wma", MTP_FORMAT_WMA},
        {".aac", MTP_FORMAT_AAC},
        {".flac", MTP_FORMAT_FLAC}
    };

    MtpObjectFormat guess_object_format(std::string extension)
    {
        std::map<std::string, MtpObjectFormat>::iterator it;

        it = formats.find(extension);
        if (it == formats.end()) {
            std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
            it = formats.find(extension);
            if (it == formats.end()) {
                return MTP_FORMAT_UNDEFINED;
            }
        }

        return it->second;
    }

    static void eraseChildRef(std::vector<MtpObjectHandle>& vec, MtpObjectHandle handle)
    {
        vec.erase(std::remove(vec.begin(), vec.end(), handle), vec.end());
    }

    static bool isObjectPropertySupported(MtpObjectProperty property)
    {
        switch (property) {
            case MTP_PROPERTY_STORAGE_ID:
            case MTP_PROPERTY_PARENT_OBJECT:
            case MTP_PROPERTY_OBJECT_FORMAT:
            case MTP_PROPERTY_OBJECT_SIZE:
            case MTP_PROPERTY_OBJECT_FILE_NAME:
            case MTP_PROPERTY_DISPLAY_NAME:
            case MTP_PROPERTY_PERSISTENT_UID:
            case MTP_PROPERTY_ASSOCIATION_TYPE:
            case MTP_PROPERTY_DATE_MODIFIED:
            case MTP_PROPERTY_ASSOCIATION_DESC:
            case MTP_PROPERTY_PROTECTION_STATUS:
            case MTP_PROPERTY_DATE_CREATED:
            case MTP_PROPERTY_HIDDEN:
            case MTP_PROPERTY_NON_CONSUMABLE:
                return true;
            default:
                return false;
        }
    }

    static bool isDebugOnlyMtpFile(const path& p)
    {
        const std::string name = p.filename().string();
        return name == ".smm_title_id" || name == "mods_status_cache.txt";
    }

    bool shouldExposePath(const path& p) const
    {
        return show_debug_mtp_files || !isDebugOnlyMtpFile(p);
    }

    void insertEntry(MtpObjectHandle handle, const DbEntry& entry)
    {
        db.insert(std::pair<MtpObjectHandle, DbEntry>(handle, entry));
        children_by_parent[entry.parent].push_back(handle);
    }

    void moveEntryParent(MtpObjectHandle handle, MtpObjectHandle new_parent)
    {
        auto it = db.find(handle);
        if (it == db.end())
            return;
        MtpObjectHandle old_parent = it->second.parent;
        if (old_parent == new_parent)
            return;
        auto vit = children_by_parent.find(old_parent);
        if (vit != children_by_parent.end())
            eraseChildRef(vit->second, handle);
        it->second.parent = new_parent;
        children_by_parent[new_parent].push_back(handle);
    }

    void eraseEntryRecursive(MtpObjectHandle handle)
    {
        auto it = db.find(handle);
        if (it == db.end())
            return;

        auto childIt = children_by_parent.find(handle);
        if (childIt != children_by_parent.end()) {
            auto children = childIt->second;
            for (auto child : children) {
                eraseEntryRecursive(child);
            }
            children_by_parent.erase(handle);
        }

        auto parentIt = children_by_parent.find(it->second.parent);
        if (parentIt != children_by_parent.end()) {
            eraseChildRef(parentIt->second, handle);
        }
        db.erase(it);
    }

    void add_file_entry(path p, MtpObjectHandle parent, MtpStorageID storage)
    {
        if (!shouldExposePath(p))
            return;

        MtpObjectHandle handle = counter;
        DbEntry entry;

        counter++;

        try {
            if (is_directory(p)) {
                entry.storage_id = storage;
                entry.parent = parent;
                entry.display_name = std::string(p.filename().string());
                entry.path = p.string();
                entry.object_format = MTP_FORMAT_ASSOCIATION;
                entry.object_size = 0;

                struct stat result;
                stat(p.string().c_str(), &result);
                entry.last_modified = result.st_mtime;
                insertEntry(handle, entry);

            } else {
                try {
                    entry.storage_id = storage;
                    entry.parent = parent;
                    entry.display_name = std::string(p.filename().string());
                    entry.path = p.string();
                    entry.object_format = MTP_FORMAT_UNDEFINED;
                    entry.object_size = file_size(p);
                    struct stat result;
                    stat(p.string().c_str(), &result);
                    entry.last_modified = result.st_mtime;

                    VLOG(1) << "Adding \"" << p.string() << "\"";

                    insertEntry(handle, entry);
                } catch (const filesystem_error& ex) {
                    LOG(WARNING) << "There was an error reading file properties";
                }
            }
        } catch (const filesystem_error& ex) {
            LOG(ERROR) << ex.what();
        }
    }

    void parse_directory(path p, MtpObjectHandle parent, MtpStorageID storage)
    {
        DbEntry entry;
        std::vector<path> v;

        // XXX Hack: This shouldn't happen but whaterver
        if(!is_directory(p))
        {
            add_file_entry(p, parent, storage);
            if (db.find(parent) != db.end())
                db.at(parent).scanned = true;
            return;
        }

        directory_iterator i(p);

        copy(i, directory_iterator(), std::back_inserter(v));

        for (std::vector<path>::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
        {
            if (!shouldExposePath(*it))
                continue;
            add_file_entry(*it, parent, storage);
        }

        if (db.find(parent) != db.end())
            db.at(parent).scanned = true;
    }

    void readFiles(const std::string& sourcedir, const std::string& display, MtpStorageID storage, bool hidden)
    {
        path p (sourcedir);
        DbEntry entry;
        MtpObjectHandle handle = counter++;
        std::string display_name = std::string(p.filename().string());

        if (!display.empty())
            display_name = display;

        try {
            if (exists(p)) {
                if (is_directory(p)) {
                    entry.storage_id = storage;
                    entry.parent = hidden ? MTP_PARENT_ROOT : 0;
                    entry.display_name = display_name;
                    entry.path = p.string();
                    entry.object_format = MTP_FORMAT_ASSOCIATION;
                    entry.object_size = 0;
                    struct stat result;
                    stat(p.string().c_str(), &result);
                    entry.last_modified = result.st_mtime;

                    insertEntry(handle, entry);

                    parse_directory (p, hidden ? 0 : handle, storage);
                } else
                    LOG(WARNING) << p << " is not a directory.";
            } else {
                if (storage == MTP_STORAGE_FIXED_RAM)
                    LOG(WARNING) << p << " does not exist.";
                else {
                    entry.storage_id = storage;
                    entry.parent = -1;
                    entry.display_name = display_name;
                    entry.path = p.parent_path().string();
                    entry.object_format = MTP_FORMAT_ASSOCIATION;
                    entry.object_size = 0;
                    entry.last_modified = 0;
                }
            }
        }
        catch (const filesystem_error& ex) {
            LOG(ERROR) << ex.what();
        }
    }

public:

    explicit SwitchMtpDatabase(bool showDebugMtpFiles = false) :
      show_debug_mtp_files(showDebugMtpFiles),
      counter(1)
    {
        local_server = nullptr;
        db = std::map<MtpObjectHandle, DbEntry>();
    }

    virtual ~SwitchMtpDatabase() {
    }

    virtual bool isHandleValid(MtpObjectHandle handle) {
        return db.find(handle) != db.end();
    }

    virtual void addStoragePath(const MtpString& path,
                                const MtpString& displayName,
                                MtpStorageID storage,
                                bool hidden)
    {
        readFiles(path, displayName, storage, hidden);
    }

    virtual void removeStorage(MtpStorageID storage)
    {
        // remove all database entries corresponding to said storage.
        for(std::map<MtpObjectHandle, DbEntry>::iterator it = db.begin(); it != db.end();) {
            if (it->second.storage_id == storage)
                it = db.erase(it);
            else
                it++;
        }
    }

    // called from SendObjectInfo to reserve a database entry for the incoming file
    virtual MtpObjectHandle beginSendObject(
        const MtpString& path,
        MtpObjectFormat format,
        MtpObjectHandle parent,
        MtpStorageID storage,
        uint64_t size,
        time_t modified)
    {
        DbEntry entry;
        MtpObjectHandle handle = counter;

        if (storage == MTP_STORAGE_FIXED_RAM && parent == 0)
            return kInvalidObjectHandle;
        if (!shouldExposePath(std::filesystem::path(path)))
            return kInvalidObjectHandle;

        VLOG(1) << __PRETTY_FUNCTION__ << ": " << path << " - " << parent
                << " format: " << std::hex << format << std::dec;

        entry.storage_id = storage;
        entry.parent = parent;
        entry.display_name = std::filesystem::path(path).filename().string();
        entry.path = path;
        entry.object_format = (format == MTP_FORMAT_ASSOCIATION) ? format : MTP_FORMAT_UNDEFINED;
        entry.object_size = size;
        entry.last_modified = modified;

        insertEntry(handle, entry);

        counter++;

        return handle;
    }

    virtual void updateObjectSize(MtpObjectHandle handle, uint64_t size)
    {
        auto it = db.find(handle);
        if (it != db.end())
            it->second.object_size = static_cast<size_t>(size);
    }

    // called to report success or failure of the SendObject file transfer
    // success should signal a notification of the new object's creation,
    // failure should remove the database entry created in beginSendObject
    virtual void endSendObject(
        const MtpString& path,
        MtpObjectHandle handle,
        MtpObjectFormat format,
        bool succeeded)
    {
        VLOG(1) << __PRETTY_FUNCTION__ << ": " << path;

        try
        {
            if (!succeeded) {
                eraseEntryRecursive(handle);
            } else {
                std::filesystem::path p (path);

                if (format != MTP_FORMAT_ASSOCIATION) {
                    auto it = db.find(handle);
                    if (it != db.end() && it->second.object_size == 0xFFFFFFFFULL) {
                        it->second.object_size = file_size(p);
                    }
                }
            }
        } catch(...)
        {
            LOG(ERROR) << __PRETTY_FUNCTION__
                       << ": failed to complete object creation:" << path;
        }
    }

    virtual MtpObjectHandleList* getObjectList(
        MtpStorageID storageID,
        MtpObjectFormat format,
        MtpObjectHandle parent)
    {
        VLOG(1) << __PRETTY_FUNCTION__ << ": " << storageID << ", " << format << ", " << parent;
        MtpObjectHandleList* list = nullptr;

        if (parent == MTP_PARENT_ROOT)
            parent = 0;

        // Scan unscanned directories
        else if (isHandleValid(parent) && !db.at(parent).scanned)
            parse_directory (db.at(parent).path, parent, storageID);

        try
        {
            std::vector<MtpObjectHandle> keys;

            auto pit = children_by_parent.find(parent);
            if (pit != children_by_parent.end()) {
                const auto& children = pit->second;
                for (auto h : children) {
                    auto dit = db.find(h);
                    if (dit == db.end())
                        continue;
                    if (dit->second.storage_id != storageID)
                        continue;
                    if (format == 0 || dit->second.object_format == format)
                        keys.push_back(h);
                }
            }

            list = new MtpObjectHandleList(keys);
        } catch(...)
        {
            list = new MtpObjectHandleList();
        }

        return list;
    }

    virtual int getNumObjects(
        MtpStorageID storageID,
        MtpObjectFormat format,
        MtpObjectHandle parent)
    {
        VLOG(1) << __PRETTY_FUNCTION__ << ": " << storageID << ", " << format << ", " << parent;

        int result = 0;

        try
        {
            MtpObjectHandleList *list = getObjectList(storageID, format, parent);
            result = list->size();
            delete list;
        } catch(...)
        {
        }

        return result;
    }

    // callee should delete[] the results from these
    // results can be NULL
    virtual MtpObjectFormatList* getSupportedPlaybackFormats()
    {
        VLOG(1) << __PRETTY_FUNCTION__;
        static const MtpObjectFormatList list = {
            MTP_FORMAT_UNDEFINED,
            MTP_FORMAT_ASSOCIATION, // folders
        };

        return new MtpObjectFormatList{list};
    }

    virtual MtpObjectFormatList* getSupportedCaptureFormats()
    {
        VLOG(1) << __PRETTY_FUNCTION__;
        static const MtpObjectFormatList list = {MTP_FORMAT_UNDEFINED, MTP_FORMAT_ASSOCIATION};
        return new MtpObjectFormatList{list};
    }

    virtual MtpObjectPropertyList* getSupportedObjectProperties(MtpObjectFormat format)
    {
        VLOG(1) << __PRETTY_FUNCTION__;
        /*
        if (format != MTP_FORMAT_PNG)
            return nullptr;
        */

        static const MtpObjectPropertyList list =
        {
            MTP_PROPERTY_STORAGE_ID,
            MTP_PROPERTY_PARENT_OBJECT,
            MTP_PROPERTY_OBJECT_FORMAT,
            MTP_PROPERTY_OBJECT_SIZE,
            MTP_PROPERTY_OBJECT_FILE_NAME,
            MTP_PROPERTY_DISPLAY_NAME,
            MTP_PROPERTY_PERSISTENT_UID,
            MTP_PROPERTY_ASSOCIATION_TYPE,
            MTP_PROPERTY_DATE_MODIFIED,

        };

        return new MtpObjectPropertyList{list};
    }

    virtual MtpDevicePropertyList* getSupportedDeviceProperties()
    {
        VLOG(1) << __PRETTY_FUNCTION__;
        static const MtpDevicePropertyList list = {
            MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME,
            MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER,
        };
        return new MtpDevicePropertyList{list};
    }

    virtual MtpResponseCode getObjectPropertyValue(
        MtpObjectHandle handle,
        MtpObjectProperty property,
        MtpDataPacket& packet)
    {
        char date[20];

        VLOG(1) << __PRETTY_FUNCTION__
                << " handle: " << handle
                << " property: " << MtpDebug::getObjectPropCodeName(property);

        if (handle == MTP_PARENT_ROOT || handle == 0)
            return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

        if (!isObjectPropertySupported(property))
            return MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED;

        try {
            switch(property)
            {
                case MTP_PROPERTY_STORAGE_ID: packet.putUInt32(db.at(handle).storage_id); break;
                case MTP_PROPERTY_PARENT_OBJECT: packet.putUInt32(db.at(handle).parent); break;
                case MTP_PROPERTY_OBJECT_FORMAT: packet.putUInt16(db.at(handle).object_format); break;
                case MTP_PROPERTY_OBJECT_SIZE: packet.putUInt32(db.at(handle).object_size); break;
                case MTP_PROPERTY_DISPLAY_NAME: packet.putString(db.at(handle).display_name.c_str()); break;
                case MTP_PROPERTY_OBJECT_FILE_NAME: packet.putString(db.at(handle).display_name.c_str()); break;
                case MTP_PROPERTY_PERSISTENT_UID: packet.putUInt128(handle); break;
                case MTP_PROPERTY_ASSOCIATION_TYPE:
                    if (db.at(handle).object_format == MTP_FORMAT_ASSOCIATION)
                        packet.putUInt16(MTP_ASSOCIATION_TYPE_GENERIC_FOLDER);
                    else
                        packet.putUInt16(0);
                    break;
                case MTP_PROPERTY_ASSOCIATION_DESC: packet.putUInt32(0); break;
                case MTP_PROPERTY_PROTECTION_STATUS:
                    packet.putUInt16(0x0000); // no files are read-only for now.
                    break;
                case MTP_PROPERTY_DATE_CREATED:
                    formatDateTime(0, date, sizeof(date));
                    packet.putString(date);
                    break;
                case MTP_PROPERTY_DATE_MODIFIED:
                    formatDateTime(db.at(handle).last_modified, date, sizeof(date));
                    packet.putString(date);
                    break;
                case MTP_PROPERTY_HIDDEN: packet.putUInt16(0); break;
                case MTP_PROPERTY_NON_CONSUMABLE:
                    if (db.at(handle).object_format == MTP_FORMAT_ASSOCIATION)
                        packet.putUInt16(0); // folders are non-consumable
                    else
                        packet.putUInt16(1); // files can usually be played.
                    break;
                default: return MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED; break;
            }

            return MTP_RESPONSE_OK;
        }
        catch (...) {
            LOG(ERROR) << __PRETTY_FUNCTION__
                       << "Could not retrieve property: "
                       << MtpDebug::getObjectPropCodeName(property)
                       << " for handle: " << handle;
            return MTP_RESPONSE_GENERAL_ERROR;
        }
    }

    virtual MtpResponseCode setObjectPropertyValue(
        MtpObjectHandle handle,
        MtpObjectProperty property,
        MtpDataPacket& packet)
    {
        DbEntry entry;
        MtpStringBuffer buffer;
        std::string oldname;
        std::string newname;
        path oldpath;
        path newpath;

        VLOG(1) << __PRETTY_FUNCTION__
                << " handle: " << handle
                << " property: " << MtpDebug::getObjectPropCodeName(property);

        if (handle == MTP_PARENT_ROOT || handle == 0)
            return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

        switch(property)
        {
            case MTP_PROPERTY_OBJECT_FILE_NAME:
                try {
                    entry = db.at(handle);

                    packet.getString(buffer);
                    newname = strdup(buffer);

                    oldpath /= entry.path;
                    newpath /= oldpath.parent_path() / newname; //TODO: compare with branch_path

                    rename(oldpath, newpath);

                    db.at(handle).display_name = newname;
                    db.at(handle).path = newpath.string();
                } catch (filesystem_error& fe) {
                    LOG(ERROR) << fe.what();
                    return MTP_RESPONSE_DEVICE_BUSY;
                } catch (std::exception& e) {
                    LOG(ERROR) << e.what();
                    return MTP_RESPONSE_GENERAL_ERROR;
                } catch (...) {
                    LOG(ERROR) << "An unexpected error has occurred";
                    return MTP_RESPONSE_GENERAL_ERROR;
                }

                break;
            case MTP_PROPERTY_PARENT_OBJECT:
                try {
                    entry = db.at(handle);
                    entry.parent = packet.getUInt32();
                }
                catch (...) {
                    LOG(ERROR) << "Could not change parent object for handle "
                               << handle;
                    return MTP_RESPONSE_GENERAL_ERROR;
                }
            default: return MTP_RESPONSE_OPERATION_NOT_SUPPORTED; break;
        }

        return MTP_RESPONSE_OK;
    }

    virtual MtpResponseCode getDevicePropertyValue(
        MtpDeviceProperty property,
        MtpDataPacket& packet)
    {
        VLOG(1) << __PRETTY_FUNCTION__;
        switch(property)
        {
            case MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER:
                packet.putString("");
                break;
            case MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME:
                packet.putString("Simple Mod Manager MTP");
                break;
            default: return MTP_RESPONSE_OPERATION_NOT_SUPPORTED; break;
        }

        return MTP_RESPONSE_OK;
    }

    virtual MtpResponseCode setDevicePropertyValue(
        MtpDeviceProperty property,
        MtpDataPacket& packet)
    {
        VLOG(1) << __PRETTY_FUNCTION__;
        return MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
    }

    virtual MtpResponseCode resetDeviceProperty(
        MtpDeviceProperty property)
    {
        VLOG(1) << __PRETTY_FUNCTION__;
        return MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
    }

    virtual MtpResponseCode getObjectPropertyList(
        MtpObjectHandle handle,
        uint32_t format,
        uint32_t property,
        int groupCode,
        int depth,
        MtpDataPacket& packet)
    {
        std::vector<MtpObjectHandle> handles;

        VLOG(2) << __PRETTY_FUNCTION__;

        if (handle == kInvalidObjectHandle)
            return MTP_RESPONSE_PARAMETER_NOT_SUPPORTED;

        if (property == 0 && groupCode == 0)
            return MTP_RESPONSE_PARAMETER_NOT_SUPPORTED;

        if (groupCode != 0)
            return MTP_RESPONSE_SPECIFICATION_BY_GROUP_UNSUPPORTED;

        if (depth > 1)
            return MTP_RESPONSE_SPECIFICATION_BY_DEPTH_UNSUPPORTED;

        if (property != ALL_PROPERTIES
                && !isObjectPropertySupported(static_cast<MtpObjectProperty>(property))) {
            return MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED;
        }

        if (depth == 0) {
            /* For a depth search, a handle of 0 is valid (objects at the root)
             * but it isn't when querying for the properties of a single object.
             */
            if (db.find(handle) == db.end())
                return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

            handles.push_back(handle);
        } else {
            auto pit = children_by_parent.find(handle);
            if (pit != children_by_parent.end()) {
                handles.reserve(pit->second.size());
                for (auto child : pit->second) {
                    auto dit = db.find(child);
                    if (dit == db.end())
                        continue;
                    if (format == 0 || dit->second.object_format == format)
                        handles.push_back(child);
                }
            }
        }

        /*
         * getObjectPropList returns an ObjectPropList dataset table;
         * built as such:
         *
         * 1- Number of elements (quadruples)
         * a1- Element 1 Object Handle
         * a2- Element 1 Property Code
         * a3- Element 1 Data type
         * a4- Element 1 Value
         * b... rinse, repeat.
         */

        static constexpr uint32_t kAllPropertyCount = 9;
        if (property == ALL_PROPERTIES)
             packet.putUInt32(kAllPropertyCount * handles.size());
        else
             packet.putUInt32(1 * handles.size());

        for(std::vector<MtpObjectHandle>::iterator it = handles.begin(); it != handles.end(); ++it) {
            MtpObjectHandle i = *it;
            const DbEntry& entry = db.at(i);

            // Persistent Unique Identifier.
            if (property == ALL_PROPERTIES || property == MTP_PROPERTY_PERSISTENT_UID) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_PERSISTENT_UID);
                packet.putUInt16(MTP_TYPE_UINT128);
                packet.putUInt128(i);
            }

            // Storage ID
            if (property == ALL_PROPERTIES || property == MTP_PROPERTY_STORAGE_ID) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_STORAGE_ID);
                packet.putUInt16(MTP_TYPE_UINT32);
                packet.putUInt32(entry.storage_id);
            }

            // Parent
            if (property == ALL_PROPERTIES || property == MTP_PROPERTY_PARENT_OBJECT) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_PARENT_OBJECT);
                packet.putUInt16(MTP_TYPE_UINT32);
                packet.putUInt32(entry.parent);
            }

            // Object Format
            if (property == ALL_PROPERTIES || property == MTP_PROPERTY_OBJECT_FORMAT) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_OBJECT_FORMAT);
                packet.putUInt16(MTP_TYPE_UINT16);
                packet.putUInt16(entry.object_format);
            }

            // Object Size
            if (property == ALL_PROPERTIES || property == MTP_PROPERTY_OBJECT_SIZE) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_OBJECT_SIZE);
                packet.putUInt16(MTP_TYPE_UINT32);
                packet.putUInt32(entry.object_size);
            }

            // Object File Name
            if (property == ALL_PROPERTIES || property == MTP_PROPERTY_OBJECT_FILE_NAME) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_OBJECT_FILE_NAME);
                packet.putUInt16(MTP_TYPE_STR);
                packet.putString(entry.display_name.c_str());
            }

            // Display Name
            if (property == ALL_PROPERTIES || property == MTP_PROPERTY_DISPLAY_NAME) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_DISPLAY_NAME);
                packet.putUInt16(MTP_TYPE_STR);
                packet.putString(entry.display_name.c_str());
            }

            // Association Type
            if (property == ALL_PROPERTIES || property == MTP_PROPERTY_ASSOCIATION_TYPE) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_ASSOCIATION_TYPE);
                packet.putUInt16(MTP_TYPE_UINT16);
                if (entry.object_format == MTP_FORMAT_ASSOCIATION)
                    packet.putUInt16(MTP_ASSOCIATION_TYPE_GENERIC_FOLDER);
                else
                    packet.putUInt16(0);
            }

            // Association Description
            if (property == MTP_PROPERTY_ASSOCIATION_DESC) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_ASSOCIATION_DESC);
                packet.putUInt16(MTP_TYPE_UINT32);
                packet.putUInt32(0);
            }

            // Protection Status
            if (property == MTP_PROPERTY_PROTECTION_STATUS) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_PROTECTION_STATUS);
                packet.putUInt16(MTP_TYPE_UINT16);
                packet.putUInt16(0x0000); //FIXME: all files are read-write for now
                // packet.putUInt16(0x8001);
            }

            // Date Created
            if (property == MTP_PROPERTY_DATE_CREATED) {
                char date[20];
                formatDateTime(0, date, sizeof(date));
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_DATE_CREATED);
                packet.putUInt16(MTP_TYPE_STR);
                packet.putString(date);
            }

            // Date Modified
            if (property == ALL_PROPERTIES || property == MTP_PROPERTY_DATE_MODIFIED) {
                char date[20];
                formatDateTime(entry.last_modified, date, sizeof(date));
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_DATE_MODIFIED);
                packet.putUInt16(MTP_TYPE_STR);
                packet.putString(date);
            }

            // Hidden
            if (property == MTP_PROPERTY_HIDDEN) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_HIDDEN);
                packet.putUInt16(MTP_TYPE_UINT16);
                packet.putUInt16(0);
            }

            // Non Consumable
            if (property == MTP_PROPERTY_NON_CONSUMABLE) {
                packet.putUInt32(i);
                packet.putUInt16(MTP_PROPERTY_NON_CONSUMABLE);
                packet.putUInt16(MTP_TYPE_UINT16);
                if (entry.object_format == MTP_FORMAT_ASSOCIATION)
                    packet.putUInt16(0); // folders are non-consumable
                else
                    packet.putUInt16(1); // files can usually be played.
            }

        }

        return MTP_RESPONSE_OK;
    }

    virtual MtpResponseCode getObjectInfo(
        MtpObjectHandle handle,
        MtpObjectInfo& info)
    {
        VLOG(2) << __PRETTY_FUNCTION__;

        if (handle == 0 || handle == MTP_PARENT_ROOT)
            return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

        try {
            info.mHandle = handle;
            info.mStorageID = db.at(handle).storage_id;
            info.mFormat = db.at(handle).object_format;
            info.mProtectionStatus = 0x0;
            info.mCompressedSize = db.at(handle).object_size;
            info.mImagePixWidth = 0;
            info.mImagePixHeight = 0;
            info.mImagePixDepth = 0;
            info.mParent = db.at(handle).parent;
            info.mAssociationType
                = info.mFormat == MTP_FORMAT_ASSOCIATION
                    ? MTP_ASSOCIATION_TYPE_GENERIC_FOLDER : 0;
            info.mAssociationDesc = 0;
            info.mSequenceNumber = 0;
            info.mName = ::strdup(db.at(handle).display_name.c_str());
            info.mDateCreated = 0;
            info.mDateModified = db.at(handle).last_modified;
            info.mKeywords = ::strdup("ubuntu,touch");

            if (VLOG_IS_ON(2))
                info.print();

            return MTP_RESPONSE_OK;
        }
        catch (...) {
            return MTP_RESPONSE_GENERAL_ERROR;
        }
    }

    virtual void* getThumbnail(MtpObjectHandle handle, size_t& outThumbSize)
    {
        outThumbSize = 0;
        return nullptr;
    }

    virtual MtpResponseCode getObjectFilePath(
        MtpObjectHandle handle,
        MtpString& outFilePath,
        int64_t& outFileLength,
        MtpObjectFormat& outFormat)
    {
        VLOG(1) << __PRETTY_FUNCTION__ << " handle: " << handle;

        if (handle == 0 || handle == MTP_PARENT_ROOT)
            return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

        try {
            DbEntry entry = db.at(handle);

            VLOG(2) << __PRETTY_FUNCTION__
                    << "handle: " << handle
                    << "path: " << entry.path
                    << "length: " << entry.object_size
                    << "format: " << entry.object_format;

            outFilePath = std::string(entry.path);
            outFileLength = entry.object_size;
            outFormat = entry.object_format;

            return MTP_RESPONSE_OK;
        }
        catch (...) {
            return MTP_RESPONSE_GENERAL_ERROR;
        }
    }

    virtual MtpResponseCode deleteFile(MtpObjectHandle handle)
    {
        size_t orig_size = db.size();
        size_t new_size;

        VLOG(2) << __PRETTY_FUNCTION__ << " handle: " << handle;

        if (handle == 0 || handle == MTP_PARENT_ROOT)
            return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

        try {
            if (db.find(handle) != db.end()) {
                eraseEntryRecursive(handle);
                return MTP_RESPONSE_OK;
            }
            else
                return MTP_RESPONSE_GENERAL_ERROR;
        }
        catch (...) {
            return MTP_RESPONSE_GENERAL_ERROR;
        }
    }

    virtual MtpResponseCode moveFile(MtpObjectHandle handle, MtpObjectHandle new_parent)
    {
        VLOG(1) << __PRETTY_FUNCTION__ << " handle: " << handle
                << " new parent: " << new_parent;

        if (handle == 0 || handle == MTP_PARENT_ROOT)
            return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

        try {
            // change parent
            moveEntryParent(handle, new_parent);
        }
        catch (...) {
            return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
        }

        return MTP_RESPONSE_OK;
    }

    /*
    virtual MtpResponseCode copyFile(MtpObjectHandle handle, MtpObjectHandle new_parent)
    {
        VLOG(2) << __PRETTY_FUNCTION__;

        // duplicate DbEntry
        // change parent

        return MTP_RESPONSE_OK
    }
    */

    virtual MtpObjectHandleList* getObjectReferences(MtpObjectHandle handle)
    {
        VLOG(1) << __PRETTY_FUNCTION__;

        if (handle == 0 || handle == MTP_PARENT_ROOT)
            return nullptr;

        return getObjectList(db.at(handle).storage_id,
                             db.at(handle).object_format,
                             handle);
    }

    virtual MtpResponseCode setObjectReferences(
        MtpObjectHandle handle,
        MtpObjectHandleList* references)
    {
        VLOG(1) << __PRETTY_FUNCTION__;

        // ignore, we don't keep the references in a list.

        return MTP_RESPONSE_OK;
    }

    virtual MtpProperty* getObjectPropertyDesc(
        MtpObjectProperty property,
        MtpObjectFormat format)
    {
        VLOG(1) << __PRETTY_FUNCTION__ << MtpDebug::getObjectPropCodeName(property);

        MtpProperty* result = nullptr;
        switch(property)
        {
            case MTP_PROPERTY_STORAGE_ID: result = new MtpProperty(property, MTP_TYPE_UINT32, false); break;
            case MTP_PROPERTY_PARENT_OBJECT: result = new MtpProperty(property, MTP_TYPE_UINT32, true); break;
            case MTP_PROPERTY_OBJECT_FORMAT: result = new MtpProperty(property, MTP_TYPE_UINT16, false); break;
            case MTP_PROPERTY_OBJECT_SIZE: result = new MtpProperty(property, MTP_TYPE_UINT32, false); break;
            case MTP_PROPERTY_WIDTH: result = new MtpProperty(property, MTP_TYPE_UINT32, false); break;
            case MTP_PROPERTY_HEIGHT: result = new MtpProperty(property, MTP_TYPE_UINT32, false); break;
            case MTP_PROPERTY_IMAGE_BIT_DEPTH: result = new MtpProperty(property, MTP_TYPE_UINT32, false); break;
            case MTP_PROPERTY_DISPLAY_NAME: result = new MtpProperty(property, MTP_TYPE_STR, true); break;
            case MTP_PROPERTY_OBJECT_FILE_NAME: result = new MtpProperty(property, MTP_TYPE_STR, true); break;
            case MTP_PROPERTY_PERSISTENT_UID: result = new MtpProperty(property, MTP_TYPE_UINT128, false); break;
            case MTP_PROPERTY_ASSOCIATION_TYPE: result = new MtpProperty(property, MTP_TYPE_UINT16, false); break;
            case MTP_PROPERTY_ASSOCIATION_DESC: result = new MtpProperty(property, MTP_TYPE_UINT32, false); break;
            case MTP_PROPERTY_PROTECTION_STATUS: result = new MtpProperty(property, MTP_TYPE_UINT16, false); break;
            case MTP_PROPERTY_DATE_CREATED: result = new MtpProperty(property, MTP_TYPE_STR, false); break;
            case MTP_PROPERTY_DATE_MODIFIED: result = new MtpProperty(property, MTP_TYPE_STR, false); break;
            case MTP_PROPERTY_HIDDEN: result = new MtpProperty(property, MTP_TYPE_UINT16, false); break;
            case MTP_PROPERTY_NON_CONSUMABLE: result = new MtpProperty(property, MTP_TYPE_UINT16, false); break;
            default: break;
        }

        return result;
    }

    virtual MtpProperty* getDevicePropertyDesc(MtpDeviceProperty property)
    {
        VLOG(1) << __PRETTY_FUNCTION__ << MtpDebug::getDevicePropCodeName(property);

        MtpProperty* result = nullptr;
        switch(property)
        {
            case MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER:
            case MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME:
                result = new MtpProperty(property, MTP_TYPE_STR, false); break;
            default: break;
        }

        return result;
    }

    virtual void sessionStarted(MtpServer* server)
    {
        VLOG(1) << __PRETTY_FUNCTION__;
        local_server = server;
    }

    virtual void sessionEnded()
    {
        VLOG(1) << __PRETTY_FUNCTION__;
        VLOG(1) << "objects in db at session end: " << db.size();
        local_server = nullptr;
    }
};
}

#endif // STUB_MTP_DATABASE_H_
