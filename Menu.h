#pragma once

#include "Entity.h"
#include "MemoryScanner.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>
#include <regex>
#include <cstdlib>

extern GhostSystems::MemoryScanner* g_Scanner;

namespace GhostSystems {

    class Menu {
    public:
        Menu(GameState& state, FeatureConfig& config) : sharedState(state), featureConfig(config) {
            initStyle();
        }

        ~Menu() {
            if (scannerStarted && ::g_Scanner) {
                ::g_Scanner->stop();
                scannerStarted = false;
            }
        }

        void render();
        void setVisible(bool visible) {
            std::lock_guard<std::mutex> lock(visibilityMutex);
            if (visible && !isVisible && !scannerStarted && ::g_Scanner) {
                ::g_Scanner->start();
                scannerStarted = true;
            } else if (!visible && isVisible && scannerStarted && ::g_Scanner) {
                ::g_Scanner->stop();
                scannerStarted = false;
            }
            isVisible = visible;
        }
        bool getVisible() const {
            return isVisible;
        }

        void OnMainThreadTick(); // Executed on Unity Main Thread

    private:
        std::mutex visibilityMutex;

    private:
        void initStyle();
        void drawEntityList();
        void drawFilters();
        void drawDebugPlayer();
        void drawESP();
        void drawIl2CppObject(void* obj, void* klass, const char* name, int depth, const std::string& path);

        GameState& sharedState;
        FeatureConfig& featureConfig;
        bool isVisible = true;
        void* selectedEntityObj = nullptr;
        
        // Flag de produção vs desenvolvimento
        bool isDebugMode = false; // Mude para false para esconder as abas de debug

        // Master Switch
        bool masterSwitch = false;
        bool scannerStarted = false;

        // Filtros da UI
        bool filterAliveOnly = true;
        bool filterHumansOnly = false;
        float maxDistanceFilter = 1000.0f;
        int filterTeamId = -1; // -1 significa sem filtro

        // Configurações de ESP
        bool espEnabled = true;
        bool espBox = false;
        int espBoxMode = 1; // 0 = Box Padrão (Sólida), 1 = Outline (Contorno)
        bool espName = true;
        bool espDistance = true;
        bool espHealth = true;
        bool espLine = true;
        bool espSkeleton = false;
        float espMaxDistance = 300.0f;
        
        // Configurações de Aimbot
        bool aimbotEnabled = true;
        int aimbotMode = 0; // 0 = Tradicional (Ao Atirar), 1 = Aimlock (Sempre)
        bool aimbotDrawFov = true;
        bool aimbotTargetAllies = true;
        float aimbotFov = 200.0f;
        int aimbotTimeMs = 0; // Tempo em milissegundos para puxar a mira
        float aimbotTransitionTimeMs = 0.0f; // Tempo para transição para a cabeça (Rage < 50, Safe > 50)
        float aimbotTransitionCurve = 2.0f; // Curva de aceleração
        bool aimbotVisibilityCheck = false; // Só puxa se o player estiver visível (DESATIVADO TEMPORARIAMENTE)
        bool aimbotMagnetic = false; // Mira Magnética (Puxa o inimigo pra frente da mira)
        std::unordered_map<void*, float> aimbotTargetTimeMap; // Guarda o tempo de foco por entidade

        // Variaveis de Debug Aimbot
        bool aimbotHasTarget = false;
        std::string aimbotTargetName = "Nenhum";
        float aimbotTargetDistFOV = 0.0f;
        float aimbotTargetDist3D = 0.0f;
        float aimbotCamPosX = 0.0f, aimbotCamPosY = 0.0f, aimbotCamPosZ = 0.0f;
        float aimbotCamRotX = 0.0f, aimbotCamRotY = 0.0f, aimbotCamRotZ = 0.0f, aimbotCamRotW = 0.0f;
        float aimbotTargetRotX = 0.0f, aimbotTargetRotY = 0.0f, aimbotTargetRotZ = 0.0f, aimbotTargetRotW = 0.0f;
        float aimbotNewRotX = 0.0f, aimbotNewRotY = 0.0f, aimbotNewRotZ = 0.0f, aimbotNewRotW = 0.0f;
        std::string aimbotErrorLog = "Nenhum erro";

        // Variaveis de controle de tempo (Delay) do Aimbot
        float aimbotDelayTimer = 0.0f;
        bool wasAimingLastFrame = false;

        // Filtros Debug Player
        struct PotentialValue {
            std::string path;
            void* obj;
            size_t offset;
            std::string type; // "float", "int", "bool"
        };
        std::vector<PotentialValue> debugPotentialValues;
        bool hasScannedValues = false;
        
        // Filtros de busca no scanner
        char searchFilter[64] = "";
        bool filterOnlyFloats = false;
        bool filterOnlyInts = false;

        // Bypass - Network Analysis
        bool bypassEnabled = false;
        char myPlayerId[32] = "";
        char myPlayerName[64] = "";
        bool bypassAutoDetectMyData = true;
        bool bypassCaptureAll = true;
        bool bypassShowAllRequests = true;
        bool bypassShowSuspectsOnly = false;
        bool bypassLogRequests = true;
        bool bypassAutoScroll = true;
        bool bypassHexView = false;
        bool bypassJsonBeautify = true;
        bool bypassHighlightMatches = true;
        bool bypassCaseSensitive = false;

        enum RequestMethod { METHOD_GET, METHOD_POST, METHOD_PUT, METHOD_DELETE, METHOD_PATCH, METHOD_OTHER };
        enum SuspiciousLevel { SUSPECT_NONE, SUSPECT_LOW, SUSPECT_MEDIUM, SUSPECT_HIGH, SUSPECT_CRITICAL };

        struct HttpHeader {
            std::string name;
            std::string value;
        };

        struct ParsedUrl {
            std::string scheme;
            std::string domain;
            std::string path;
            std::string query;
            std::string fragment;
            int port;
        };

        struct HttpRequest {
            uint64_t id;
            std::string timestamp;
            RequestMethod method;
            std::string methodStr;
            std::string url;
            ParsedUrl parsedUrl;
            std::string endpoint;
            std::vector<HttpHeader> headers;
            std::string body;
            std::string raw;
            std::string response;
            bool isSuspect;
            SuspiciousLevel suspectLevel;
            std::string suspectReason;
            std::vector<std::string> matchedPatterns;
            std::vector<std::pair<std::string, std::string>> highlightRanges;
            int bodySize;
            int totalSize;
            float responseTimeMs;
        };

        std::vector<HttpRequest> bypassAllRequests;
        std::vector<HttpRequest> bypassSuspectRequests;
        std::vector<std::string> bypassSuspectKeywords;
        std::vector<std::string> bypassCustomKeywords;
        int bypassSelectedRequestIdx = -1;
        int bypassSelectedRequestList = 0;
        std::string bypassRequestDetails;
        char bypassSearchFilter[64];
        char bypassBodyFilter[64];
        char bypassUrlFilter[64];
        int bypassMaxRequests = 1000;
        bool bypassScrollToBottom = false;
        int bypassSortBy = 0;
        int bypassViewMode = 0;
        int bypassStatsGet = 0, bypassStatsPost = 0, bypassStatsPut = 0, bypassStatsOther = 0;
        int bypassStatsSuspect = 0;
        bool bypassStatsDirty = true;
        std::mutex bypassMutex;

        bool bypassCriticalAlert = false;
        int bypassCriticalCount = 0;
        char bypassMethodFilter[16] = "";
        char bypassBodySearch[64] = "";
        bool bypassSearchBody = false;
        bool bypassHighlightPlayerData = true;
        bool bypassDecodeGzip = true;
        bool bypassCollapseDetails = false;
        int bypassSelectedDetailTab = 0;
        bool bypassAutoExport = false;
        char bypassAutoExportPath[256] = "/sdcard/Download/GhostSystems_Auto.log";
        bool bypassLogToFile = true;
        int bypassRequestIdCounter = 0;

        bool bypassUnityWebRequestHooked = false;
        void* orig_SendWebRequest = nullptr;
        void* orig_ReceiveCallback = nullptr;
        bool bypassAutoDetectEnabled = true;
        bool bypassScanInProgress = false;
        bool bypassScanComplete = false;
        bool bypassPlayerDataFound = false;
        char bypassDetectedPlayerId[32];
        char bypassDetectedPlayerName[64];

        bool bypassUseRegex = true;
        bool bypassSqliteEnabled = false;
        bool bypassSqliteInitialized = false;
        void* bypassSqliteDb = nullptr;

        int bypassRequestsPerSecond = 0;
        int bypassRequestsLastMinute = 0;
        float bypassDashboardData[60];
        int bypassDashboardIndex = 0;

        void drawBypass();
        void drawBypassConfig();
        void drawBypassStats();
        void drawBypassList();
        void drawBypassDetails();
        void checkRequestForSuspicious(HttpRequest& req);
        bool containsPlayerData(const std::string& text);
        void addBypassRequest(const HttpRequest& req);
        void parseUrl(HttpRequest& req);
        void formatJson(std::string& json);
        std::string bytesToHex(const std::string& data);
        void calculateBypassStats();
        void exportRequestToFile(const HttpRequest& req, const char* filepath);
        void exportAllToFile(const char* filepath);

        void* getUnityWebRequestSendInternal();
        void* getUnityWebRequestReceiveCallback();
        bool hookUnityWebRequest();
        void unhookUnityWebRequest();
        bool isUnityWebRequestHooked();
        std::string decompressGzip(const std::string& data);
        std::string decompressDeflate(const std::string& data);
        bool scanMemoryForPlayerData();
        bool scanForPlayerId(const std::string& regionName);
        bool scanForPlayerName(const std::string& regionName);
        bool matchRegex(const std::string& pattern, const std::string& text);
        bool initSqlite();
        void closeSqlite();
        bool exportToSqlite(const HttpRequest& req);
        bool exportAllToSqlite();
        void drawBypassDashboard();

        void scanForPotentialValues(void* obj, void* klass, const std::string& path, int depth, std::unordered_set<void*>& visited);
        void drawEntityProperties(void* obj, int depth = 0);

        static void hook_UnityWebRequest_SendWebRequest(void* unityWebRequest);
        static void hook_UnityWebRequest_ReceiveCallback(void* unityWebRequest, void* operationHandle);
        static HttpRequest& getCurrentCapturedRequest();
        static void setCurrentCapturedRequest(const HttpRequest& req);
        static thread_local HttpRequest t_CurrentCapturedRequest;
    };

} // namespace GhostSystems
