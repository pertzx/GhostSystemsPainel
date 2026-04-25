#pragma once
#include <dlfcn.h>
#include <string.h>

// Simple Il2Cpp bindings
typedef void* (*il2cpp_domain_get_t)();
typedef void** (*il2cpp_domain_get_assemblies_t)(const void* domain, size_t* size);
typedef const char* (*il2cpp_image_get_name_t)(void* image);
typedef void* (*il2cpp_assembly_get_image_t)(const void* assembly);
typedef void* (*il2cpp_class_from_name_t)(const void* image, const char* namespaze, const char* name);
typedef void* (*il2cpp_class_get_field_from_name_t)(void* klass, const char* name);
typedef void* (*il2cpp_class_get_method_from_name_t)(void* klass, const char* name, int argsCount);
typedef const char* (*il2cpp_class_get_name_t)(void* klass);
typedef void* (*il2cpp_class_get_fields_t)(void* klass, void** iter);
typedef void* (*il2cpp_class_get_methods_t)(void* klass, void** iter);
typedef const char* (*il2cpp_method_get_name_t)(const void* method);
typedef uint32_t (*il2cpp_method_get_param_count_t)(const void* method);
typedef void* (*il2cpp_method_get_param_t)(const void* method, uint32_t index);
typedef const char* (*il2cpp_field_get_name_t)(void* field);
typedef size_t (*il2cpp_field_get_offset_t)(void* field);
typedef void* (*il2cpp_field_get_type_t)(void* field);
typedef char* (*il2cpp_type_get_name_t)(const void* type);
typedef void* (*il2cpp_class_from_type_t)(const void* type);
typedef bool (*il2cpp_class_is_valuetype_t)(const void* klass);
typedef void (*il2cpp_field_static_get_value_t)(void* field, void* value);
typedef void (*il2cpp_field_get_value_t)(void* obj, void* field, void* value);
typedef void* (*il2cpp_runtime_invoke_t)(void* method, void* obj, void** params, void** exc);
typedef void* (*il2cpp_object_get_class_t)(void* obj);
typedef size_t (*il2cpp_array_length_t)(void* array);
typedef size_t (*il2cpp_class_get_array_element_size_t)(void* klass);

typedef void* (*il2cpp_class_get_parent_t)(void* klass);
typedef void* (*il2cpp_thread_attach_t)(void* domain);
typedef void (*il2cpp_thread_detach_t)(void* thread);

typedef void* (*il2cpp_class_get_type_t)(void* klass);
typedef void* (*il2cpp_type_get_object_t)(const void* type);
typedef void* (*il2cpp_string_new_t)(const char* str);
typedef void (*il2cpp_free_t)(void* ptr);

namespace Il2Cpp {
    inline il2cpp_domain_get_t domain_get = nullptr;
    inline il2cpp_domain_get_assemblies_t domain_get_assemblies = nullptr;
    inline il2cpp_image_get_name_t image_get_name = nullptr;
    inline il2cpp_assembly_get_image_t assembly_get_image = nullptr;
    inline il2cpp_class_from_name_t class_from_name = nullptr;
    inline il2cpp_class_get_field_from_name_t class_get_field_from_name = nullptr;
    inline il2cpp_class_get_method_from_name_t class_get_method_from_name = nullptr;
    inline il2cpp_class_get_name_t class_get_name = nullptr;
    inline il2cpp_class_get_parent_t class_get_parent = nullptr;
    inline il2cpp_class_get_fields_t class_get_fields = nullptr;
    inline il2cpp_class_get_methods_t class_get_methods = nullptr;
    inline il2cpp_method_get_name_t method_get_name = nullptr;
    inline il2cpp_method_get_param_count_t method_get_param_count = nullptr;
    inline il2cpp_method_get_param_t method_get_param = nullptr;
    inline il2cpp_field_get_name_t field_get_name = nullptr;
    inline il2cpp_field_get_offset_t field_get_offset = nullptr;
    inline il2cpp_field_get_type_t field_get_type = nullptr;
    inline il2cpp_type_get_name_t type_get_name = nullptr;
    inline il2cpp_class_from_type_t class_from_type = nullptr;
    inline il2cpp_class_is_valuetype_t class_is_valuetype = nullptr;
    inline il2cpp_field_static_get_value_t field_static_get_value = nullptr;
    inline il2cpp_field_get_value_t field_get_value = nullptr;
    inline il2cpp_runtime_invoke_t runtime_invoke = nullptr;
    inline il2cpp_object_get_class_t object_get_class = nullptr;
    inline il2cpp_array_length_t array_length = nullptr;
    inline il2cpp_class_get_array_element_size_t class_get_array_element_size = nullptr;
    inline il2cpp_thread_attach_t thread_attach = nullptr;
    inline il2cpp_thread_detach_t thread_detach = nullptr;

    inline il2cpp_class_get_type_t class_get_type = nullptr;
    inline il2cpp_type_get_object_t type_get_object = nullptr;
    inline il2cpp_string_new_t string_new = nullptr;
    inline il2cpp_free_t free_func = nullptr;

    inline bool Initialize() {
        void* handle = dlopen("libil2cpp.so", RTLD_LAZY);
        if (!handle) return false;

        domain_get = (il2cpp_domain_get_t)dlsym(handle, "il2cpp_domain_get");
        domain_get_assemblies = (il2cpp_domain_get_assemblies_t)dlsym(handle, "il2cpp_domain_get_assemblies");
        image_get_name = (il2cpp_image_get_name_t)dlsym(handle, "il2cpp_image_get_name");
        assembly_get_image = (il2cpp_assembly_get_image_t)dlsym(handle, "il2cpp_assembly_get_image");
        class_from_name = (il2cpp_class_from_name_t)dlsym(handle, "il2cpp_class_from_name");
        class_get_field_from_name = (il2cpp_class_get_field_from_name_t)dlsym(handle, "il2cpp_class_get_field_from_name");
        class_get_method_from_name = (il2cpp_class_get_method_from_name_t)dlsym(handle, "il2cpp_class_get_method_from_name");
        class_get_name = (il2cpp_class_get_name_t)dlsym(handle, "il2cpp_class_get_name");
        class_get_parent = (il2cpp_class_get_parent_t)dlsym(handle, "il2cpp_class_get_parent");
        class_get_fields = (il2cpp_class_get_fields_t)dlsym(handle, "il2cpp_class_get_fields");
        class_get_methods = (il2cpp_class_get_methods_t)dlsym(handle, "il2cpp_class_get_methods");
        method_get_name = (il2cpp_method_get_name_t)dlsym(handle, "il2cpp_method_get_name");
        method_get_param_count = (il2cpp_method_get_param_count_t)dlsym(handle, "il2cpp_method_get_param_count");
        method_get_param = (il2cpp_method_get_param_t)dlsym(handle, "il2cpp_method_get_param");
        field_get_name = (il2cpp_field_get_name_t)dlsym(handle, "il2cpp_field_get_name");
        field_get_offset = (il2cpp_field_get_offset_t)dlsym(handle, "il2cpp_field_get_offset");
        field_get_type = (il2cpp_field_get_type_t)dlsym(handle, "il2cpp_field_get_type");
        type_get_name = (il2cpp_type_get_name_t)dlsym(handle, "il2cpp_type_get_name");
        class_from_type = (il2cpp_class_from_type_t)dlsym(handle, "il2cpp_class_from_type");
        class_is_valuetype = (il2cpp_class_is_valuetype_t)dlsym(handle, "il2cpp_class_is_valuetype");
        field_static_get_value = (il2cpp_field_static_get_value_t)dlsym(handle, "il2cpp_field_static_get_value");
        field_get_value = (il2cpp_field_get_value_t)dlsym(handle, "il2cpp_field_get_value");
        runtime_invoke = (il2cpp_runtime_invoke_t)dlsym(handle, "il2cpp_runtime_invoke");
        object_get_class = (il2cpp_object_get_class_t)dlsym(handle, "il2cpp_object_get_class");
        array_length = (il2cpp_array_length_t)dlsym(handle, "il2cpp_array_length");
        class_get_array_element_size = (il2cpp_class_get_array_element_size_t)dlsym(handle, "il2cpp_class_get_array_element_size");
        thread_attach = (il2cpp_thread_attach_t)dlsym(handle, "il2cpp_thread_attach");
        thread_detach = (il2cpp_thread_detach_t)dlsym(handle, "il2cpp_thread_detach");
        class_get_type = (il2cpp_class_get_type_t)dlsym(handle, "il2cpp_class_get_type");
        type_get_object = (il2cpp_type_get_object_t)dlsym(handle, "il2cpp_type_get_object");
        string_new = (il2cpp_string_new_t)dlsym(handle, "il2cpp_string_new");
        free_func = (il2cpp_free_t)dlsym(handle, "il2cpp_free");

        return domain_get != nullptr;
    }

    inline void* FindLinecastMethodSafe(void* klass, uint32_t* outParamCount) {
        if (!class_get_methods || !method_get_name || !method_get_param_count || !method_get_param || !type_get_name) {
            return nullptr;
        }
        void* curr = klass;
        while (curr) {
            void* iter = nullptr;
            while (void* method = class_get_methods(curr, &iter)) {
                if (strcmp(method_get_name(method), "Linecast") == 0) {
                    uint32_t pCount = method_get_param_count(method);
                    if (pCount >= 3) {
                        void* p0 = method_get_param(method, 0);
                        void* p1 = method_get_param(method, 1);
                        void* p2 = method_get_param(method, 2);
                        if (p0 && p1 && p2) {
                            const char* n0 = type_get_name(p0);
                            const char* n1 = type_get_name(p1);
                            const char* n2 = type_get_name(p2);
                            
                            if (n0 && n1 && n2) {
                                // Aceita Vector3, Vector3, int/Int32
                                bool ok0 = strstr(n0, "Vector3") != nullptr;
                                bool ok1 = strstr(n1, "Vector3") != nullptr;
                                bool ok2 = strstr(n2, "Int32") != nullptr || strstr(n2, "int") != nullptr;
                                
                                if (ok0 && ok1 && ok2) {
                                    if (outParamCount) *outParamCount = pCount;
                                    return method;
                                }
                            }
                        }
                    }
                }
            }
            curr = class_get_parent(curr);
        }
        return nullptr;
    }

    inline void* GetImage(const char* imageName) {
        if (!domain_get) return nullptr;
        size_t size = 0;
        void* domain = domain_get();
        void** assemblies = domain_get_assemblies(domain, &size);
        for (size_t i = 0; i < size; ++i) {
            void* image = assembly_get_image(assemblies[i]);
            if (strcmp(image_get_name(image), imageName) == 0) {
                return image;
            }
        }
        return nullptr;
    }

    inline void* GetClass(const char* imageName, const char* namespaze, const char* className) {
        void* image = GetImage(imageName);
        if (!image) return nullptr;
        return class_from_name(image, namespaze, className);
    }

    inline void* GetMethodRecursively(void* klass, const char* methodName, int argsCount) {
        if (!class_get_parent || !class_get_method_from_name) return nullptr;   
        void* curr = klass;
        while (curr) {
            void* method = class_get_method_from_name(curr, methodName, argsCount);
            if (method) return method;
            curr = class_get_parent(curr);
        }
        return nullptr;
    }

    inline void* FindMethod(void* klass, const char* methodName, int argsCount, const char** argTypes) {
        if (!class_get_methods || !method_get_name || !method_get_param_count || !method_get_param || !type_get_name) {
            return nullptr;
        }
        void* curr = klass;
        while (curr) {
            void* iter = nullptr;
            while (void* method = class_get_methods(curr, &iter)) {
                if (strcmp(method_get_name(method), methodName) == 0 && method_get_param_count(method) == argsCount) {
                    bool match = true;
                    for (int i = 0; i < argsCount; i++) {
                        void* paramType = method_get_param(method, i);
                        const char* paramTypeName = type_get_name(paramType);
                        if (!paramTypeName || strstr(paramTypeName, argTypes[i]) == nullptr) {
                            match = false;
                            break;
                        }
                    }
                    if (match) return method;
                }
            }
            curr = class_get_parent(curr);
        }
        return nullptr;
    }

    inline void* FindRaycastMethodSafe(void* klass, uint32_t* outParamCount) {
        if (!class_get_methods || !method_get_name || !method_get_param_count || !method_get_param || !type_get_name) {
            return nullptr;
        }
        void* curr = klass;
        while (curr) {
            void* iter = nullptr;
            while (void* method = class_get_methods(curr, &iter)) {
                if (strcmp(method_get_name(method), "Raycast") == 0) {
                    uint32_t pCount = method_get_param_count(method);
                    if (pCount >= 4) {
                        void* p0 = method_get_param(method, 0);
                        void* p1 = method_get_param(method, 1);
                        void* p2 = method_get_param(method, 2);
                        void* p3 = method_get_param(method, 3);
                        if (p0 && p1 && p2 && p3) {
                            const char* n0 = type_get_name(p0);
                            const char* n1 = type_get_name(p1);
                            const char* n2 = type_get_name(p2);
                            const char* n3 = type_get_name(p3);
                            
                            if (n0 && n1 && n2 && n3) {
                                // Aceita Vector3, float/Single, int/Int32
                                bool ok0 = strstr(n0, "Vector3") != nullptr;
                                bool ok1 = strstr(n1, "Vector3") != nullptr;
                                bool ok2 = strstr(n2, "Single") != nullptr || strstr(n2, "float") != nullptr;
                                bool ok3 = strstr(n3, "Int32") != nullptr || strstr(n3, "int") != nullptr;
                                
                                if (ok0 && ok1 && ok2 && ok3) {
                                    if (outParamCount) *outParamCount = pCount;
                                    return method;
                                }
                            }
                        }
                    }
                }
            }
            curr = class_get_parent(curr);
        }
        return nullptr;
    }

    inline bool IsSubclassOf(void* klass, const char* targetClassName) {
        if (!class_get_parent || !class_get_name) return false;
        void* curr = klass;
        while (curr) {
            const char* name = class_get_name(curr);
            if (name && strcmp(name, targetClassName) == 0) {
                return true;
            }
            curr = class_get_parent(curr);
        }
        return false;
    }

    // Helper to read dictionary values
    // This is specific to System.Collections.Generic.Dictionary<K, V> in Il2Cpp
    // We can find the "entries" and "count" fields dynamically.
    inline bool GetDictionaryValues(void* dictObj, void** outValues, int* outCount) {
        if (!dictObj || !object_get_class) return false;
        void* klass = object_get_class(dictObj);
        
        void* entriesField = class_get_field_from_name(klass, "_entries");
        if (!entriesField) entriesField = class_get_field_from_name(klass, "entries");
        
        void* countField = class_get_field_from_name(klass, "_count");
        if (!countField) countField = class_get_field_from_name(klass, "count");

        if (!entriesField || !countField) return false;

        int count = 0;
        field_get_value(dictObj, countField, &count);
        *outCount = count;

        void* entriesArray = nullptr;
        field_get_value(dictObj, entriesField, &entriesArray);
        if (!entriesArray) return false;

        *outValues = entriesArray; // This is an Il2CppArray*
        return true;
    }

    inline const char* Unsafe_GetPointer(void* obj) {
        if (!obj) return nullptr;
        return *(const char**)(obj);
    }
}
