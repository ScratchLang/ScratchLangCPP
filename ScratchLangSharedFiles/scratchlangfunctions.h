// ScratchlangCPP.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include <compiler.h>
#include <decompiler.h>
#include <editor.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <dirent.h>
#include <miniz/miniz.h>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <Windows.h>
#else
#include <termios.h>
#include <unistd.h>
inline std::string exec(const char *cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
#endif

// ANSI Color codes
inline extern const std::string RED = "\033[0;31m";
inline extern const std::string NC = "\033[0m";
inline extern const std::string P = "\033[0;35m";

inline std::string operator*(std::string_view const &a, unsigned int b) {
    std::string output = "";
    while (b--) {
        output += a;
    }
    return output;
}

#ifdef _WIN32
inline void add_to_zip(mz_zip_archive &zip_archive, const std::string &path,
                       const std::string &prefix = "") {
    for (auto &entry : std::filesystem::directory_iterator(path)) {
        std::string full_path = entry.path().string();
        std::string relative_path = prefix + entry.path().filename().string();
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

inline void extract_zip(const std::string &zip_path,
                        const std::string &dest_path) {
    mz_zip_archive zip_archive = {0};
    mz_bool status = mz_zip_reader_init_file(&zip_archive, zip_path.c_str(), 0);
    if (!status)
        throw std::runtime_error("Failed to open zip archive: " + zip_path);
    for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&zip_archive); i++) {
        mz_zip_archive_file_stat file_stat;
        mz_zip_reader_file_stat(&zip_archive, i, &file_stat);
        std::string file_path = dest_path + "/" + file_stat.m_filename;
        std::filesystem::create_directories(
            std::filesystem::path(file_path).parent_path());
        if (mz_zip_reader_is_file_a_directory(&zip_archive, i))
            std::filesystem::create_directory(file_path);
        else {
            void *file_data = malloc(file_stat.m_uncomp_size);
            mz_zip_reader_extract_to_mem(&zip_archive, i, file_data,
                                         file_stat.m_uncomp_size, 0);
            std::ofstream file(file_path, std::ios::binary);
            file.write(static_cast<const char *>(file_data),
                       file_stat.m_uncomp_size);
            free(file_data);
        }
    }

    mz_zip_reader_end(&zip_archive);
}

inline bool filedialog(const char *title, const char *startingpath,
                       int filtercount, const char *patterns[],
                       std::string &path) {
    try {
        path = tinyfd_openFileDialog(title, startingpath, filtercount, patterns,
                                     nullptr, 0);
        return true;
    } catch (...) { // Gives an access violation if the user presses "cancel"
                    // instead of selecting a file. In order to catch it go to
                    // "ScratchlangCPP property pages > Configuration Properties
                    // > C/C++ > Code Generation > Enable C++ Exeptions" and set
                    // that to "Yes with SEH Exceptions (/EHa)"
        return false;
    }
}
#endif

inline void error(std::string const &text) {
    std::cout << RED << "Error: " << text << NC << std::endl;
}

inline void error(const char *text) {
    std::cout << RED << "Error: " << text << NC << std::endl;
}

inline void writetofile(std::string const &file, std::string const &towrite) {
    bool exists = std::filesystem::exists(file);
    std::ofstream openfile(file, std::ios::app);
    if (exists) {
        openfile << "\n" << towrite;
        return;
    }
    openfile << towrite;
}

inline std::string getexecwd() {
    std::string out;
#ifdef _WIN32
    TCHAR szfile[MAX_PATH];
    GetModuleFileName(nullptr, szfile, MAX_PATH);
#ifndef UNICODE
    out = szFile;
#else
    std::wstring wout = szfile;
    out = std::string(wout.begin(), wout.end());
#endif
#else
    out = std::filesystem::canonical("/proc/self/exe");
#endif
    return out;
}

inline void sleep(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}