// ScratchlangCPP.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include <compiler.h>
#include <conio.h>
#include <decompiler.h>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <miniz/miniz.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <Windows.h>

using namespace std;

// ANSI Color codes
inline extern const string RED = "\033[0;31m";
inline extern const string NC = "\033[0m";
inline extern const string P = "\033[0;35m";

inline void add_to_zip(mz_zip_archive &zip_archive, const string &path,
                       const string &prefix = "") {
    for (auto &entry : filesystem::directory_iterator(path)) {
        string full_path = entry.path().string();
        string relative_path = prefix + entry.path().filename().string();
        if (entry.is_directory()) {
            relative_path += "/";
            add_to_zip(zip_archive, full_path, relative_path);
        } else {
            mz_zip_writer_add_file(&zip_archive, relative_path.c_str(),
                                   full_path.c_str(), nullptr, 0,
                                   MZ_BEST_COMPRESSION);
        }
    }
}

inline string operator*(string a, unsigned int b) {
    string output = "";
    while (b--) {
        output += a;
    }
    return output;
}

inline void extract_zip(const string &zip_path, const string &dest_path) {
    mz_zip_archive zip_archive = {0};
    mz_bool status = mz_zip_reader_init_file(&zip_archive, zip_path.c_str(), 0);
    if (!status)
        throw runtime_error("Failed to open zip archive: " + zip_path);
    for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&zip_archive); i++) {
        mz_zip_archive_file_stat file_stat;
        mz_zip_reader_file_stat(&zip_archive, i, &file_stat);
        string file_path = dest_path + "/" + file_stat.m_filename;
        filesystem::create_directories(
            filesystem::path(file_path).parent_path());
        if (mz_zip_reader_is_file_a_directory(&zip_archive, i))
            filesystem::create_directory(file_path);
        else {
            void *file_data = malloc(file_stat.m_uncomp_size);
            mz_zip_reader_extract_to_mem(&zip_archive, i, file_data,
                                         file_stat.m_uncomp_size, 0);
            ofstream file(file_path, ios::binary);
            file.write(static_cast<const char *>(file_data),
                       file_stat.m_uncomp_size);
            free(file_data);
        }
    }

    mz_zip_reader_end(&zip_archive);
}

inline void error(string text) {
    cout << RED << "Error: " << text << NC << endl;
}

inline void error(const char *text) {
    cout << RED << "Error: " << text << NC << endl;
}

inline char getinput(string message = "") {
    if (message != "")
        cout << message << endl;
    return (char)_getch();
}

inline void writetofile(string file, string towrite) {
    bool exists = filesystem::exists(file);
    ofstream openfile(file, ios::app);
    if (exists) {
        openfile << "\n" << towrite;
        return;
    }
    openfile << towrite;
}

inline string getexecwd() {
    string out;
    TCHAR szfile[MAX_PATH];
    GetModuleFileName(NULL, szfile, MAX_PATH);
#ifndef UNICODE
    out = szFile;
#else
    wstring wout = szfile;
    out = string(wout.begin(), wout.end());
#endif
    return out;
}