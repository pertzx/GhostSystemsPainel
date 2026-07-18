#pragma once

#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <android/log.h>
#include <string.h>
#include <stdio.h>
#include <thread>
#include <atomic>
#include <imgui.h>

#ifndef LOG_TAG
#define LOG_TAG "GhostSystems"
#endif

#ifndef LOGI
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#endif
#ifndef LOGE
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif
#ifndef LOGW
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#endif

namespace GhostSystems {

    class TouchReader {
    public:
        TouchReader() : isRunning(false), touchFd(-1), screenWidth(1920), screenHeight(1080), max_x(1), max_y(1) {}

        ~TouchReader() {
            stop();
        }

        void setScreenSize(int w, int h) {
            if (w > 0 && h > 0) {
                screenWidth = w;
                screenHeight = h;
            }
        }

        void start() {
            if (isRunning) return;
            isRunning = true;
            touchThread = std::thread(&TouchReader::readLoop, this);
            touchThread.detach();
        }

        void stop() {
            isRunning = false;
            if (touchFd >= 0) {
                close(touchFd);
                touchFd = -1;
            }
        }

        void updateImGui(ImGuiIO& io) {
            io.AddMousePosEvent(currentX.load(std::memory_order_relaxed), currentY.load(std::memory_order_relaxed));
            io.AddMouseButtonEvent(0, isDown.load(std::memory_order_relaxed));
        }

    private:
        std::atomic<bool> isRunning;
        int touchFd;
        std::thread touchThread;
        
        int screenWidth, screenHeight;
        int max_x, max_y;

        std::atomic<float> currentX{0.0f};
        std::atomic<float> currentY{0.0f};
        std::atomic<bool> isDown{false};

        int findTouchDevice() {
            const char* dirname = "/dev/input";
            DIR* dir = opendir(dirname);
            if (!dir) return -1;

            struct dirent* de;
            int fd = -1;
            while ((de = readdir(dir))) {
                if (strncmp(de->d_name, "event", 5) == 0) {
                    char path[256];
                    snprintf(path, sizeof(path), "%s/%s", dirname, de->d_name);
                    
                    int tempFd = open(path, O_RDONLY | O_NONBLOCK);
                    if (tempFd < 0) continue;

                    unsigned long bitmask[EV_MAX / 8 + 1] = {0};
                    ioctl(tempFd, EVIOCGBIT(0, sizeof(bitmask)), bitmask);
                    
                    // Verifica se eh um dispositivo de input absoluto (touch)
                    if (bitmask[EV_ABS / 8] & (1 << (EV_ABS % 8))) {
                        unsigned long absmask[ABS_MAX / 8 + 1] = {0};
                        ioctl(tempFd, EVIOCGBIT(EV_ABS, sizeof(absmask)), absmask);
                        
                        struct input_absinfo abs_x, abs_y;
                        bool has_mt = false;
                        bool has_abs = false;

                        if ((absmask[ABS_MT_POSITION_X / 8] & (1 << (ABS_MT_POSITION_X % 8))) &&
                            ioctl(tempFd, EVIOCGABS(ABS_MT_POSITION_X), &abs_x) >= 0 &&
                            ioctl(tempFd, EVIOCGABS(ABS_MT_POSITION_Y), &abs_y) >= 0) {
                            max_x = abs_x.maximum;
                            max_y = abs_y.maximum;
                            has_mt = true;
                        } else if ((absmask[ABS_X / 8] & (1 << (ABS_X % 8))) &&
                                   ioctl(tempFd, EVIOCGABS(ABS_X), &abs_x) >= 0 &&
                                   ioctl(tempFd, EVIOCGABS(ABS_Y), &abs_y) >= 0) {
                            max_x = abs_x.maximum;
                            max_y = abs_y.maximum;
                            has_abs = true;
                        }

                        if ((has_mt || has_abs) && max_x > 0 && max_y > 0) {
                            LOGI("Found touch device: %s (max_x: %d, max_y: %d)", path, max_x, max_y);
                            fd = tempFd;
                            break;
                        }
                    }
                    close(tempFd);
                }
            }
            closedir(dir);
            return fd;
        }

        void readLoop() {
            // Tenta abrir repetidamente ate encontrar
            int attempts = 0;
            while (isRunning && touchFd < 0) {
                touchFd = findTouchDevice();
                if (touchFd < 0) {
                    attempts++;
                    if (attempts % 10 == 0) {
                        LOGI("TouchReader: procurando device... (tentativa %d)", attempts);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }

            if (!isRunning) return;

            if (touchFd >= 0) {
                LOGI("TouchReader: device encontrado, fd=%d, max_x=%d, max_y=%d", touchFd, max_x, max_y);
            } else {
                LOGW("TouchReader: nenhum touch device encontrado, touch desabilitado");
                return;
            }

            struct input_event ev;
            int x = 0, y = 0;
            
            while (isRunning) {
                int bytes = read(touchFd, &ev, sizeof(ev));
                if (bytes == sizeof(ev)) {
                    if (ev.type == EV_ABS) {
                        if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X) {
                            x = ev.value;
                            currentX = (float)x / max_x * screenWidth;
                        } else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y) {
                            y = ev.value;
                            currentY = (float)y / max_y * screenHeight;
                        } else if (ev.code == ABS_MT_TRACKING_ID) {
                            isDown = (ev.value != -1);
                        } else if (ev.code == ABS_MT_PRESSURE || ev.code == ABS_PRESSURE) {
                            isDown = (ev.value > 0);
                        }
                    } else if (ev.type == EV_KEY) {
                        if (ev.code == BTN_TOUCH || ev.code == BTN_TOOL_FINGER || ev.code == BTN_LEFT) {
                            isDown = (ev.value != 0);
                        }
                    } else if (ev.type == EV_SYN) {
                        // Synced event
                    }
                } else {
                    // Evita uso de 100% da CPU se o FD estiver em non-blocking
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                }
            }
        }
    };
}
