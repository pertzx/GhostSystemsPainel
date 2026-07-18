#pragma once
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include "skStr.h"
#include "json.hpp"
using json = nlohmann::json;

// Função alternativa para verificar se arquivo existe
bool file_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

std::string ReadFromJson(std::string path, std::string section) {
    if (!file_exists(path))
        return skCrypt("File Not Found").decrypt();
    
    std::ifstream file(path);
    if (!file.is_open())
        return skCrypt("File Open Error").decrypt();
    
    try {
        json data = json::parse(file);
        return data[section];
    } catch (...) {
        return skCrypt("JSON Parse Error").decrypt();
    }
}

bool CheckIfJsonKeyExists(std::string path, std::string section) {
    if (!file_exists(path))
        return false;
    
    std::ifstream file(path);
    if (!file.is_open())
        return false;
    
    try {
        json data = json::parse(file);
        return data.contains(section);
    } catch (...) {
        return false;
    }
}

bool WriteToJson(std::string path, std::string name, std::string value, bool userpass, std::string name2, std::string value2) {
    json file;
    if (!userpass) {
        file[name] = value;
    } else {
        file[name] = value;
        file[name2] = value2;
    }

    std::ofstream jsonfile(path, std::ios::out);
    if (!jsonfile.is_open())
        return false;
    
    jsonfile << file;
    jsonfile.close();
    
    return file_exists(path);
}

// Função de verificação de autenticação para Android
void checkAuthenticated(const std::string& ownerid) {
    while (true) {
        // Verificação alternativa para Android
        // Pode ser substituída por outra verificação de sua escolha
        if (access(skCrypt("/data/data/com.dts.freefireth"), F_OK) != 0) {
            exit(13);
        }
        sleep(1); // thread interval (usando sleep do unistd.h)
    }
}
