#include <cstring>
#include <jni.h>
#include <pthread.h>
#include <cstdio>
#include <unistd.h>
#include <dlfcn.h>
#include <list>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <linux/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *genv) override {
        this->api = api;
        this->env = env;
		env = genv;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) {
            LOGD(OBFUSCATE("Skip unknown process"));
            return;
        }
        enable_hack = isGame(env, args->app_data_dir);
    }
private:
    Api *api;
    JNIEnv *env;

    void preSpecialize(const char *process) {
        unsigned r = 0;
        int fd = api->connectCompanion();
        read(fd, &r, sizeof(r));
        close(fd);
        LOGD(OBFUSCATE("process=[%s], r=[%u]\n"), process, r);
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

};

static int urandom = -1;

static void companion_handler(int i) {
    if (urandom < 0) {
        urandom = open(OBFUSCATE("/dev/urandom"), O_RDONLY);
    }
    unsigned r;
    read(urandom, &r, sizeof(r));
    LOGD(OBFUSCATE("companion r=[%u]\n"), r);
    write(i, &r, sizeof(r));
}


int isGame(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir)
        return 0;
    const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
    int user = 0;
    static char package_name[256];
    if (sscanf(app_data_dir, OBFUSCATE("/data/%*[^/]/%d/%s"), &user, package_name) != 2) {
        if (sscanf(app_data_dir, OBFUSCATE("/data/%*[^/]/%s"), package_name) != 1) {
            package_name[0] = '\0';
            LOGW("can't parse %s", app_data_dir);
            return 0;
        }
    }
    if (strcmp(package_name, OBFUSCATE("com.dts.freefireth")) == 0) {
        LOGI(OBFUSCATE("detect game: %s"), package_name);
        game_data_dir = new char[strlen(app_data_dir) + 1];
        strcpy(game_data_dir, app_data_dir);
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 1;
    } else {
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 0;
    }
}
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

uintptr_t parse_maps_for_lib(const char* maps_buf, const char* libname) {
    // Duyệt từng dòng tìm dòng chứa libname, format: "address-perms offset dev inode pathname..."
    const char* p = maps_buf;
    while (p && *p) {
        const char* line_end = strchr(p, '\n');
        size_t line_len = line_end ? (size_t)(line_end - p) : strlen(p);

        if (line_len > 0) {
            // làm 1 bản copy tạm để xử lý an toàn
            char line[512];
            size_t copy_len = line_len < sizeof(line)-1 ? line_len : sizeof(line)-1;
            memcpy(line, p, copy_len);
            line[copy_len] = '\0';

            if (strstr(line, libname)) {
                // dòng thường bắt đầu bằng "hexaddr-hexaddr"
                char* dash = strchr(line, '-');
                if (dash) {
                    *dash = '\0';
                    // parse base address
                    uintptr_t addr = (uintptr_t)strtoull(line, NULL, 16);
                    return addr;
                }
            }
        }

        if (!line_end) break;
        p = line_end + 1;
    }

    return 0;
}

uintptr_t getRealOffset(uintptr_t offset) {
    // static để cache persist suốt lifetime process
    static uintptr_t base = 0;

    // nếu đã lấy được base, trả luôn (vẫn thêm noise page nhỏ)
    if (base != 0) {
        uintptr_t page_mask = ~(sysconf(_SC_PAGESIZE) - 1);
        uintptr_t random_noise = (uintptr_t)(rand() % 0x1000) & ~(page_mask);
        return (base & page_mask) + offset + random_noise;
    }

    // Khởi tạo RNG 1 lần
    static bool rand_init = false;
    if (!rand_init) {
        srand((unsigned)time(NULL) ^ (unsigned)getpid());
        rand_init = true;
    }

    // 1) Thử dlopen trước (ít I/O, hợp lệ để resolve base khi library được load)
    void* handle = dlopen("libil2cpp.so", RTLD_NOW | RTLD_NOLOAD);
    if (!handle) {
        // nếu không được với NOLOAD, thử mở bình thường (không ideal nhưng là fallback)
        handle = dlopen("libil2cpp.so", RTLD_NOW | RTLD_LOCAL);
    }
    if (handle) {
        base = (uintptr_t)handle;
    }

    // 2) Nếu dlopen không trả base hợp lý, đọc /proc/self/maps **1 lần**
    if (base == 0) {
        const char* maps_path = "/proc/self/maps";
        int fd = open(maps_path, O_RDONLY);
        if (fd >= 0) {
            // đọc tối đa 64KB maps — thường đủ cho chuỗi maps
            const size_t BUF_SZ = 10 * 1024;
            char* buf = (char*)malloc(BUF_SZ);
            if (buf) {
                ssize_t r = read(fd, buf, BUF_SZ - 1);
                if (r > 0) {
                    buf[r] = '\0';
                    uintptr_t parsed = parse_maps_for_lib(buf, "libil2cpp.so");
                    if (parsed != 0) {
                        base = parsed;
                    }
                }
                free(buf);
            }
            close(fd);
        } else {
            // lỗi mở file maps — có thể quyền/metadata, ghi log nếu cần
            // perror("open /proc/self/maps");
        }
    }

    // 3) Nếu vẫn 0, thử dlopen lần nữa (không fatal)
    if (base == 0) {
        void* handle2 = dlopen("libil2cpp.so", RTLD_NOW | RTLD_LOCAL);
        if (handle2) base = (uintptr_t)handle2;
    }

    // 4) Nếu vẫn không tìm được base, trả offset fallback (không an toàn, caller phải check)
    if (base == 0) {
        // tùy vào use-case bạn có thể chọn crash, throw, hoặc trả 0 để báo lỗi
        // ở đây trả offset nguyên thủy để caller thấy kết quả không đúng
        return offset;
    }

    // cuối cùng trả giá trị với page-aligned base + random_noise nhỏ
    uintptr_t page_mask = ~(sysconf(_SC_PAGESIZE) - 1);
    uintptr_t random_noise = (uintptr_t)(rand() % 0x1000) & ~(page_mask);
    return (base & page_mask) + offset + random_noise;
}
