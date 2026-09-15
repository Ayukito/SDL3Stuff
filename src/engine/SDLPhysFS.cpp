#include "SDLPhysFS.h"

namespace SDLPhysFS{
     namespace{
        // char* file_read(const char* filename) {
        //     SDL_IOStream *rw = SDL_IOFromFile(filename, "rb");
        //     if (rw == NULL) return NULL;

        //     Sint64 res_size = SDL_GetIOSize(rw);
        //     char* res = (char*)malloc(res_size + 1);

        //     Sint64 nb_read_total = 0, nb_read = 1;
        //     char* buf = res;
        //     while (nb_read_total < res_size && nb_read != 0) {
        //         nb_read = SDL_ReadIO(rw, buf, (res_size - nb_read_total));
        //         nb_read_total += nb_read;
        //         buf += nb_read;
        //     }
        //     SDL_CloseIO(rw);
        //     if (nb_read_total != res_size) {
        //         free(res);
        //         return NULL;
        //     }

        //     res[nb_read_total] = '\0';
        //     return res;
        // }
    }

    std::string readFile(const std::string& path){
        PHYSFS_File* fp = PHYSFS_openRead(path.c_str());
        std::string output;

        if (fp){
            PHYSFS_sint64 len = PHYSFS_fileLength(fp);
            if (len > 0){
                output.reserve(static_cast<size_t>(len));
            }

            char buffer[4096];
            PHYSFS_sint64 rc;
            do {
                rc = PHYSFS_readBytes(fp, buffer, sizeof(buffer));
                if (rc < 0) {
                    PHYSFS_close(fp);
                    return {};
                }

                output.append(buffer, static_cast<size_t>(rc));
            } while (rc == static_cast<PHYSFS_sint64>(sizeof(buffer)));
            PHYSFS_close(fp);
        }

        return output;
    }

    std::string mountAssets(const std::string& assetsPath){
#ifdef SDL_PLATFORM_ANDROID
        const char* zipName = "Assets.zip";

        size_t dataSize = 0;
        void* data = SDL_LoadFile(zipName, &dataSize);
        if (data == nullptr){
            SDL_Log("SDLPhysFS: could not read %s from APK assets! SDL error: %s", zipName, SDL_GetError());
            return {};
        }

        if (PHYSFS_mountMemory(data, static_cast<PHYSFS_uint64>(dataSize), SDL_free, zipName, "/", 1) == 0){
            SDL_Log("PhysFS Error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
            SDL_free(data);
            return {};
        }

        return zipName;
#else
        if (PHYSFS_mount(assetsPath.c_str(), "/", 1) == 0){
            SDL_Log("PhysFS Error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
            return {};
        }

        return assetsPath;
#endif
    }

    bool dummyRead(){
        // char* file = file_read("Assets/test.txt");
        // SDL_Log("file: %s", file);
        //ToDo: Use SDL RWops on Android instead of physfs, make filesystem wrapper. Use PHYSFS_mountMemory if using .zip to mount
        // Assumes the caller has already mounted the Assets directory (see main()).
        bool testFileExists = false;

        testFileExists = PHYSFS_exists("/test.txt");
        SDL_Log("Exists: %d", testFileExists?1:0);

        std::string txt = readFile("/test.txt");
        SDL_Log("Text: %s", txt.c_str());

        return testFileExists;
    }
}