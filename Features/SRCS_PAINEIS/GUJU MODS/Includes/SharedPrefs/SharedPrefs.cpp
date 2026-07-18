//
// Created by Administrator on 08/12/2024.
//

#include "SharedPrefs.h"
SharedPrefs prefs("prefs.txt");

std::string SharedPrefsGetPackageName() {
    char *application_id[256];
    FILE *fp = fopen("proc/self/cmdline", "r");
    if (fp) {
        fread(application_id, sizeof(application_id), 1, fp);
        fclose(fp);
        return std::string((const char*)application_id);
    }
    return "";
}

void SharedPrefs::load() {
    static std::string package_name = SharedPrefsGetPackageName();
    std::ifstream file(("/data/user/0/" + package_name + "/" + filename));
    if (file.is_open()) {
        std::string key, value;
        while (getline(file, key) && getline(file, value)) {
            data[key] = value;
        }
        file.close();
    }
}

void SharedPrefs::save() {
    static std::string package_name = SharedPrefsGetPackageName();
    std::ofstream file(("/data/user/0/" + package_name + "/" + filename));
    if (file.is_open()) {
        for (const auto& pair : data) {
            file << pair.first << std::endl;
            file << pair.second << std::endl;
        }
        file.close();
    }
}

// Constructor nhận vào tên file
SharedPrefs::SharedPrefs(const std::string& file) : filename(file) {
    load();  // Khi tạo đối tượng, đọc dữ liệu từ file
}

// Lưu một giá trị văn bản với một khóa
void SharedPrefs::putString(const std::string& key, const std::string& value) {
    data[key] = value;
    save();  // Sau khi thay đổi, lưu lại vào file
}

std::string SharedPrefs::getString(const std::string& key, const std::string& defaultValue) {
    if (data.find(key) != data.end()) {
        return data[key];
    } else {
        return defaultValue;
    }
}

// Xóa một khóa
void SharedPrefs::remove(const std::string& key) {
    data.erase(key);
    save();  // Sau khi xóa, lưu lại vào file
}
