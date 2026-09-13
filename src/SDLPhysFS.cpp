#include "SDLPhysFS.h"

namespace SDLPhysFS{

    string PHYSFS_readFile( string path){
        string output;
        PHYSFS_File* fp;
        fp = PHYSFS_openRead(path.c_str());
        if (fp){
            //char buffer[PHYSFS_fileLength(fp)];
            char buffer[128];
            PHYSFS_sint64 rc;
            do {
                rc = PHYSFS_readBytes(fp, buffer, sizeof(buffer));
                string s(buffer);
                output += s.substr(0, rc);
            } while (!(rc < sizeof(buffer)));
        }
        PHYSFS_close(fp);

        return output;
    }

    char* file_read(const char* filename) {
        SDL_IOStream *rw = SDL_IOFromFile(filename, "rb");
        if (rw == NULL) return NULL;

        Sint64 res_size = SDL_GetIOSize(rw);
        char* res = (char*)malloc(res_size + 1);

        Sint64 nb_read_total = 0, nb_read = 1;
        char* buf = res;
        while (nb_read_total < res_size && nb_read != 0) {
            nb_read = SDL_ReadIO(rw, buf, (res_size - nb_read_total));
            nb_read_total += nb_read;
            buf += nb_read;
        }
        SDL_CloseIO(rw);
        if (nb_read_total != res_size) {
            free(res);
            return NULL;
        }

        res[nb_read_total] = '\0';
        return res;
    }

    bool PhysFS_DummyRead(){
        int res;

        SDL_Log("Base Path: %s", GetBasePath().c_str());
        char* file = file_read("Assets/test.txt");
        SDL_Log("file: %s", file);
        //ToDo: Use SDL RWops on Android instead of physfs, make filesystem wrapper. Use PHYSFS_mountMemory if using .zip to mount
        string tmp = GetBasePath() + "Assets";
        char *array = &tmp[0];
        res = PHYSFS_mount(array, "/", 1);
        SDL_Log("Mounted: %d", res);
        if (res == 0){
            SDL_Log("PhysFS Error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        }

        bool exists = false;

        exists = PHYSFS_exists("/test.txt");
        SDL_Log("Exists: %d", exists?1:0);

        string txt = PHYSFS_readFile("/test.txt");
        SDL_Log("Text: %s", txt.c_str());

        return exists;
    };

}