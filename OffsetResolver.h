#pragma once
#include "Il2CppHelper.h"
#include <unordered_map>
#include <string>
#include <mutex>
#include <cstring>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "OffsetResolver", __VA_ARGS__)

namespace GhostSystems {

class OffsetResolver {
public:
    static OffsetResolver& Instance() {
        static OffsetResolver instance;
        return instance;
    }

    // Get field offset by name (searches common player/weapon classes)
    size_t GetPlayerFieldOffset(const char* fieldName) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        
        // Check cache first
        auto it = fieldOffsetCache.find(fieldName);
        if (it != fieldOffsetCache.end()) {
            return it->second;
        }
        
        size_t offset = FindFieldOffset(fieldName);
        if (offset > 0) {
            fieldOffsetCache[fieldName] = offset;
            return offset;
        }
        
        // Try aliases for common field names
        if (strcmp(fieldName, "weapon") == 0) {
            offset = FindFieldOffset("ActiveUISightingWeapon");
            if (offset > 0) {
                fieldOffsetCache[fieldName] = offset;
                return offset;
            }
        }
        
        return 0;
    }

    size_t GetWeaponFieldOffset(const char* fieldName) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        
        auto it = weaponFieldOffsetCache.find(fieldName);
        if (it != weaponFieldOffsetCache.end()) {
            return it->second;
        }
        
        size_t offset = FindWeaponFieldOffset(fieldName);
        if (offset > 0) {
            weaponFieldOffsetCache[fieldName] = offset;
        }
        return offset;
    }

    // Clear all caches
    void ClearCache() {
        std::lock_guard<std::mutex> lock(cacheMutex);
        fieldOffsetCache.clear();
        weaponFieldOffsetCache.clear();
        methodCache.clear();
        classCache.clear();
    }

private:
    OffsetResolver() {}
    
    std::mutex cacheMutex;
    std::unordered_map<std::string, size_t> fieldOffsetCache;
    std::unordered_map<std::string, size_t> weaponFieldOffsetCache;
    std::unordered_map<std::string, void*> methodCache;
    std::unordered_map<std::string, void*> classCache;

    // Common player class names to search
    const char* playerClasses[4] = {
        "Player", "PlayerCharacter", "Soldier", "Character"
    };
    
    const char* playerNamespaces[3] = {
        "COW.GamePlay", "COW", ""
    };

    // Common weapon class names
    const char* weaponClasses[6] = {
        "Weapon", "GunController", "WeaponComponent", "WeaponBase", "FDAEPHMIEPC", "WeaponFireComponent"
    };

    size_t FindFieldOffset(const char* fieldName) {
        // Try multiple class/namespace combinations
        for (const char* ns : playerNamespaces) {
            for (const char* cls : playerClasses) {
                void* klass = GetCachedClass("Assembly-CSharp.dll", ns, cls);
                if (!klass) continue;
                
                size_t offset = FindFieldInClass(klass, fieldName);
                if (offset > 0) {
                    LOGI("[OffsetResolver] Found field '%s' in %s::%s at 0x%zX", 
                         fieldName, ns ? ns : "", cls, offset);
                    return offset;
                }
                
                // Also search parent classes
                void* parent = Il2Cpp::class_get_parent(klass);
                while (parent) {
                    offset = FindFieldInClass(parent, fieldName);
                    if (offset > 0) {
                        LOGI("[OffsetResolver] Found field '%s' in parent of %s::%s at 0x%zX", 
                             fieldName, ns ? ns : "", cls, offset);
                        return offset;
                    }
                    parent = Il2Cpp::class_get_parent(parent);
                }
            }
        }
        return 0;
    }

    size_t FindWeaponFieldOffset(const char* fieldName) {
        for (const char* ns : playerNamespaces) {
            for (const char* cls : weaponClasses) {
                void* klass = GetCachedClass("Assembly-CSharp.dll", ns, cls);
                if (!klass) continue;
                
                size_t offset = FindFieldInClass(klass, fieldName);
                if (offset > 0) {
                    LOGI("[OffsetResolver] Found weapon field '%s' in %s::%s at 0x%zX", 
                         fieldName, ns ? ns : "", cls, offset);
                    return offset;
                }
                
                void* parent = Il2Cpp::class_get_parent(klass);
                while (parent) {
                    size_t offset = FindFieldInClass(parent, fieldName);
                    if (offset > 0) return offset;
                    parent = Il2Cpp::class_get_parent(parent);
                }
            }
        }
        return 0;
    }

    size_t FindFieldInClass(void* klass, const char* fieldName) {
        if (!klass || !Il2Cpp::class_get_fields) return 0;
        
        void* iter = nullptr;
        void* field = nullptr;
        while ((field = Il2Cpp::class_get_fields(klass, &iter)) != nullptr) {
            const char* name = Il2Cpp::field_get_name(field);
            if (name && strcmp(name, fieldName) == 0) {
                return Il2Cpp::field_get_offset(field);
            }
        }
        return 0;
    }

    void* GetCachedClass(const char* assembly, const char* namespaze, const char* className) {
        std::string key = std::string(assembly) + "|" + (namespaze ? namespaze : "") + "|" + className;
        
        auto it = classCache.find(key);
        if (it != classCache.end()) {
            return it->second;
        }
        
        void* klass = Il2Cpp::GetClass(assembly, namespaze, className);
        if (klass) {
            classCache[key] = klass;
        }
        return klass;
    }
};

// Global instance accessor
inline OffsetResolver& g_OffsetResolver() {
    return OffsetResolver::Instance();
}

} // namespace GhostSystems