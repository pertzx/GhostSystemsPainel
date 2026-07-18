#ifndef LOGIN_H
#define LOGIN_H

#include "StrEnc.h"
#include "Includes.h"
#include <curl/curl.h>
#include "Tools.h"
#include "json.hpp"
#include "Log.h"
#include "LicenseTools.h"
#include <jni.h>
#include <string>
#include "obfuscate.h"
#include <sys/system_properties.h>

using json = nlohmann::ordered_json;
bool bValid = false;
std::string g_Auth, g_Token,EXP;
bool check;
bool bear;
time_t rng = 0;

#endif
