/*
    Author: Daniel (dmilith) Dettlaff
    © 2011-2013 - VerKnowSys
*/


#include "shellutils.h"


const char* gatherUserNameFromDirEntry(int uid, const char* users_home_dir) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(users_home_dir)) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0)) continue;
            // only if owner of given directory is set to current runtime uid…
            struct stat buffer;
            int status;
            char *fname = ent->d_name;
            stringstream hd;
            stringstream hd2;
            hd << fname;
            hd2 << users_home_dir << "/" << fname;
            status = stat(hd2.str().c_str(), &buffer);
            if (buffer.st_uid == uid) {
                #ifdef DEVEL
                    cerr << "UID EQUALS current uid for " << fname << endl;
                #endif
                closedir(dir);
                return hd.str().c_str();
            }
            #ifdef DEVEL
                cerr << "UID of file: " << fname << " is " << buffer.st_uid << endl;
            #endif
        }
        closedir(dir);
        return NULL;
    } else {
        /* could not open directory */
        perror ("");
        cerr << "Failed to find required home directory. Cannot continue" << endl;
        exit(EXIT_FAILURE);
    }
}
