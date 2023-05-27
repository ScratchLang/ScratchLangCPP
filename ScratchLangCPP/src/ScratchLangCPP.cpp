// ScratchlangCPP.cpp : Defines the entry point for the application.
//
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <scratchlangfunctions.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#ifdef _WIN32
#define CURL_STATICLIB
#include <atlstr.h>
#include <conio.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Options.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>
#endif
#ifdef linux
#include <unistd.h>
#endif

using namespace std;

void inputloop(string, char *[], bool, string const &);

static size_t writedata(void const *ptr, size_t size, size_t nmemb,
                        FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

class WriterMemoryClass {
public:
    // Helper Class for reading result from remote host
    WriterMemoryClass() {
        this->m_pBuffer = nullptr;
        this->m_pBuffer = (char *)malloc(20000 * sizeof(char));
        this->m_Size = 0;
    };

    ~WriterMemoryClass() {
        if (this->m_pBuffer)
            free(this->m_pBuffer);
    };

    void *Realloc(void *ptr, size_t size) {
        if (ptr)
            return realloc(ptr, size);
        else
            return malloc(size);
    };

    // Callback must be declared static, otherwise it won't link...
    size_t WriteMemoryCallback(char const *ptr, size_t size, size_t nmemb) {
        // Calculate the real size of the incoming buffer
        size_t realsize = size * nmemb;

        // (Re)Allocate memory for the buffer
        m_pBuffer = (char *)Realloc(m_pBuffer, m_Size + realsize);

        // Test if Buffer is initialized correctly & copy memory
        if (m_pBuffer == nullptr) {
            realsize = 0;
        }

        memcpy(&(m_pBuffer[m_Size]), ptr, realsize);
        m_Size += realsize;

        // return the real size of the buffer...
        return realsize;
    };

    void print() const {
        cout << "Size: " << m_Size << endl;
        cout << "Content: " << endl << m_pBuffer << endl;
    }

    // Public member vars
    char *m_pBuffer;
    size_t m_Size;
};

void startloop(const char *a1 = "", int argus = 0, char *arguv[] = {}) {
    filesystem::path p = getexecwd();
    string realcwd = p.parent_path().string();
    filesystem::current_path((filesystem::path)(realcwd + "/data"));
    fstream f("version", ios::in);
    string ver;
    getline(f, ver);
    f.close();
    filesystem::current_path("mainscripts");
    if (!filesystem::exists("var/asked") && !filesystem::exists("var/vc")) {
        if (tolower(getinput("Would you like ScratchLang to check its version "
                             "every time you start it? [Y/N]")) == 'y')
            writetofile("var/vc", "Don't remove this file please.");
        writetofile("var/asked", "Don't remove this file please.");
    }
    if (filesystem::exists("var/vc") && a1 != "nope") {
        cout << "Checking version..." << endl;
        if (filesystem::exists("version"))
            filesystem::remove("version");
#ifdef _WIN32
        curlpp::Cleanup clean;
        curlpp::Easy request;
        WriterMemoryClass mWriterChunk;
        curlpp::types::WriteFunctionFunctor wfunctor =
            bind(&WriterMemoryClass::WriteMemoryCallback, &mWriterChunk,
                 placeholders::_1, placeholders::_2, placeholders::_3);
        request.setOpt<curlpp::options::Url>(
            "https://raw.githubusercontent.com/ScratchLang/ScratchLangCPP/"
            "master/ScratchLangCPP/data/version");
        request.setOpt<curlpp::options::Verbose>(false);
        request.setOpt<curlpp::options::NoProgress>(true);
        request.setOpt<curlpp::options::WriteFunction>(wfunctor);
#endif
#ifdef linux
        system("wget -q "
               "https://raw.githubusercontent.com/ScratchLang/ScratchLangCPP/"
               "master/ScratchLangCPP/data/version");
#endif
        try {
#ifdef _WIN32
            FILE *file = fopen("version", "wb");
            if (file) {
                request.setOpt<curlpp::options::WriteFile>(file);
                request.perform();
                fclose(file);
            }
#endif
            string utd = "1";
            string gver;
            f = fstream("version", ios::in);
            getline(f, gver);
            f.close();
            filesystem::remove("version");
            if (ver != gver)
                utd = "0";
            if (utd == "0" &&
                tolower(getinput("Your version of ScratchLang (" + ver +
                                 ") is outdated. The current version is " +
                                 gver + ". Would you like to update? [Y/N]")) ==
                    'y') {
                system("git pull origin main");
                exit(0);
            }
        } catch (...) {
            error("Checking version failed.");
        }
    }
    bool args = false;
    bool args2 = false;
    if (argus > 1) {
        if (strcmp(arguv[1], "--help") == 0) {
#ifdef _WIN32
            cout << "Usage: scratchlang.exe [OPTIONS]\n"
                 << endl
                 << "  -1                Create a project." << endl
                 << "  -2                Remove a project." << endl
                 << "  -3                Compile a project." << endl
                 << "  -4                Decompile a project." << endl
                 << "  -5                Export a project." << endl
                 << "  -6                Import a project." << endl
                 << "  --debug [FILE]    Debug a ScratchScript file. Currently "
                    "not "
                    "available."
                 << endl
                 << "  --help            Display this help message." << endl
                 << "  --edit            Edit a ScratchLang project.\n"
                 << endl
                 << "Examples:" << endl
                 << "  .\\scratchlang.exe -4 C:\\Users\\Me\\project.sb3"
                 << endl;
#endif
#ifdef linux
            cout << "Usage: scratchlang.exe [OPTIONS]\n"
                 << endl
                 << "  -1                Create a project." << endl
                 << "  -2                Remove a project." << endl
                 << "  -3                Compile a project." << endl
                 << "  -4                Decompile a project." << endl
                 << "  -5                Export a project." << endl
                 << "  -6                Import a project." << endl
                 << "  --debug [FILE]    Debug a ScratchScript file. Currently "
                    "not "
                    "available."
                 << endl
                 << "  --help            Display this help message." << endl
                 << "  --edit            Edit a ScratchLang project.\n"
                 << endl
                 << "Examples:" << endl
                 << "  .\\scratchlang.exe -4 C:\\Users\\Me\\project.sb3"
                 << endl;
#endif
            exit(0);
        }
        args = true;
        if (argus > 2)
            args2 = true;
    }
#ifdef _WIN32
    system("cls");
#endif
#ifdef linux
    system("clear");
#endif
    cout
        << P
        << "\n      /$$$$$$                                 /$$               "
           "/$$       /$$                                    \n"
        << "     /$$__  $$                               | $$              | "
           "$$ "
           "     | $$                                    \n"
        << "    | $$  \\__/  /$$$$$$$  /$$$$$$  /$$$$$$  /$$$$$$    /$$$$$$$| "
           "$$$$$$$ | $$        /$$$$$$  /$$$$$$$   /$$$$$$ \n"
        << "    |  $$$$$$  /$$_____/ /$$__  $$|____  $$|_  $$_/   /$$_____/| "
           "$$__  $$| $$       |____  $$| $$__  $$ /$$__  $$\n"
        << "     \\____  $$| $$      | $$  \\__/ /$$$$$$$  | $$    | $$      | "
           "$$  \\ $$| $$        /$$$$$$$| $$  \\ $$| $$  \\ $$\n"
        << "     /$$  \\ $$| $$      | $$      /$$__  $$  | $$ /$$| $$      | "
           "$$  | $$| $$       /$$__  $$| $$  | $$| $$  | $$\n"
        << "    |  $$$$$$/|  $$$$$$$| $$     |  $$$$$$$  |  $$$$/|  $$$$$$$| "
           "$$ "
           " | $$| $$$$$$$$|  $$$$$$$| $$  | $$|  $$$$$$$\n"
        << "     \\______/  \\_______/|__/      \\_______/   \\___/   "
           "\\_______/|__/  |__/|________/ \\_______/|__/  |__/ \\____  $$\n"
        << "                                                                   "
           " "
           "                                    /$$  \\ $$\n"
        << "                                                                   "
           " "
           "                                   |  $$$$$$/\n"
        << "                                                                   "
           " "
           "                                    \\______/ \n"
        << NC << endl; // print the logo
    if (!args || a1 == "nope") {
        cout << "Welcome to ScratchLang " << ver
             << " (Name suggested by @MagicCrayon9342 on Scratch.)" << endl;
        inputloop("", arguv, args2, realcwd);
    } else {
        if (strcmp(arguv[1], "--edit") == 0) {
            cout << "Sorry, the editor has not been implemented yet." << endl;
            exit(0);
        } else
            inputloop(arguv[1], arguv, args2, realcwd);
    }
    filesystem::current_path("..");
    if (filesystem::exists("projects")) {
        filesystem::current_path("projects");
        filesystem::directory_iterator diriterate(".");
        int filecount = 0;
        for (const auto &_ : filesystem::directory_iterator("."))
            filecount++;
        if (filecount == 0) {
            filesystem::current_path("..");
            filesystem::remove("projects");
        }
    }
}

void inputloop(string ia1, char *aac[] = {}, bool aab = false,
               string const &realcwd = "") {
    string inp = "";
    int a1 = NULL;
    if (ia1 != "") {
        ia1.erase(0, 1);
        try {
            a1 = stoi(ia1);
        } catch (invalid_argument &) {
            error(ia1 + " is not an argument.");
            sleep(2000);
            startloop("nope");
        }
    }
    if (a1 == NULL) {
        cout << "1) Create a project." << endl
             << "2) Remove a project." << endl
             << "3) Compile a project." << endl
             << "4) Decompile a .sb3 file." << endl
             << "5) Export project." << endl
             << "6) Import project." << endl;
        if (!filesystem::exists("var/devmode")) {
            cout << "7) Enable Developer Mode." << endl << "8) Exit." << endl;
        } else {
            cout << "7) Disable Developer Mode." << endl
                 << "8) Delete all variables." << endl
                 << "9) Prepare for commit and push." << endl
                 << "0) Exit." << endl;
        }
        inp = getinput();
    } else {
        if (a1 > 0 && a1 < 7)
            inp = to_string(a1);
        else {
            error(ia1 + " is not an argument.");
            sleep(2000);
            startloop("nope");
        }
    }
    if (inp == "1") {
        cout << "\nName your project." << endl;
        string namechar;
        getline(cin, namechar);
        string name = namechar;
        filesystem::current_path("..");
        if (namechar == "") {
            error("Project name cannot be empty.");
            exit(0);
        } else if (filesystem::exists("projects/" + name)) {
            char yessor =
                getinput("Project" + name + " already exists. Replace? [Y/N]");
            if (tolower(yessor) == 'y')
                filesystem::remove_all(namechar);
            else if (tolower(yessor) == 'n')
                exit(0);
            else {
                error(yessor + " is not an input.");
                exit(0);
            }
        }
        cout << "You named your project " + name +
                    ". If you want to rename it, use the File Explorer."
             << endl;
        filesystem::create_directories("projects/" + name + "/Stage/assets");
        filesystem::copy("resources/cd21514d0531fdffb22204e0ec5ed84a.svg",
                         "projects/" + name + "/Stage/assets",
                         filesystem::copy_options::overwrite_existing);
        writetofile("projects/" + name + "/Stage/project.ss1",
                    "// There should be no empty lines.");
        filesystem::create_directories("projects/" + name + "/Sprite1/assets");
        filesystem::copy("resources/341ff8639e74404142c11ad52929b021.svg",
                         "projects/" + name + "/Sprite1/assets",
                         filesystem::copy_options::overwrite_existing);
        writetofile("projects/" + name + "/Sprite1/project.ss1",
                    "// There should be no empty lines.");
        cout << "Sorry, the editor is not available as of this moment." << endl;
    } else if (inp == "2") {
        filesystem::current_path("..");
        if (!filesystem::exists("projects")) {
            error("There are no projects to delete.");
            exit(0);
        }
        filesystem::current_path("projects");
        cout << endl;
        for (const auto &file : filesystem::directory_iterator("."))
            cout << filesystem::canonical(file.path()) << endl;
        cout << endl;
        string ddrd;
        getline(cin, ddrd);
        filesystem::path pgrd = ddrd;
        if (pgrd != "") {
            if (filesystem::exists(pgrd))
                filesystem::remove_all(pgrd);
            else
                error("Directory " + pgrd.string() + "does not exist.");
        }
        exit(0);
    } else if (inp == "3") {
        compiler();
        exit(0);
    } else if (inp == "4") {
        decompiler(aac, aab, realcwd);
        exit(0);
    } else if (inp == "5") {
        filesystem::current_path("..");
        if (!filesystem::exists("projects")) {
            error("There are no projects to export.");
            exit(0);
        }
        filesystem::current_path("projects");
        cout << endl;
        for (const auto &file : filesystem::directory_iterator("."))
            cout << filesystem::canonical(file.path()).filename().string()
                 << endl;
        cout << "\nChoose a project to export, or input nothing to cancel."
             << endl;
        string ddrd;
        getline(cin, ddrd);
        filesystem::path pgrd = ddrd;
        if (pgrd != "") {
            if (filesystem::exists(pgrd)) {
                filesystem::current_path("../projects");
#ifdef _WIN32
                mz_zip_archive zip_archive = {0};
                mz_zip_writer_init_file(
                    &zip_archive,
                    (filesystem::current_path().parent_path().string() +
                     "/exports/" + pgrd.string() + ".ssa")
                        .c_str(),
                    0);
                add_to_zip(zip_archive, ".");
                mz_zip_writer_finalize_archive(&zip_archive);
                mz_zip_writer_end(&zip_archive);
#endif
#ifdef linux
                system(("tar -cf " +
                        filesystem::current_path().parent_path().string() +
                        "/exports/" + pgrd.string() + ".ssa " + pgrd.string())
                           .c_str());
#endif
            } else
                error("Directory " + pgrd.string() + " does not exist.");
        }
        exit(0);
    } else if (inp == "6") {
        filesystem::current_path("..");
        if (!filesystem::exists("projects"))
            filesystem::create_directory("projects");
        filesystem::current_path("mainscripts");
        const char *filterPatterns[1] = {"*.ssa"};
        string ssi;
#ifdef _WIN32
        ssi =
            tinyfd_openFileDialog("Choose a ScratchScript Archive",
                                  "../exports", 1, filterPatterns, nullptr, 0);
#endif
#ifdef linux
        ssi =
            exec("zenity --file-selection --file-filter 'ScratchScript Archive "
                 "(*.ssa) | *.ssa' --file-filter 'All Files (*.*) | *.*' "
                 "--title='Choose a ScratchScript Archive'");
#endif
        filesystem::current_path(((filesystem::path)ssi).parent_path());
#ifdef _WIN32
        filesystem::rename(ssi, "a.zip");
        extract_zip("a.zip", ".");
        filesystem::rename("a.zip", ssi);
        filesystem::path path;
        for (const auto &entry : filesystem::directory_iterator("."))
            path = filesystem::canonical(entry.path());
        filesystem::copy(path,
                         (filesystem::current_path().parent_path().string() +
                          "/projects/" + path.filename().string())
                             .c_str(),
                         filesystem::copy_options::overwrite_existing |
                             filesystem::copy_options::recursive);
        filesystem::current_path("..");
        filesystem::remove_all(((filesystem::path)ssi)
                                   .filename()
                                   .replace_extension("")
                                   .string()
                                   .c_str());
#endif
#ifdef linux
        system(("tar -xf " + ssi).c_str());
        system(
            ("cp -rf " +
             ((filesystem::path)ssi).filename().replace_extension("").string() +
             " ../projects")
                .c_str());
        filesystem::remove_all(
            ((filesystem::path)ssi).filename().replace_extension(""));
#endif
        if (tolower(getinput("Remove .ssa file? [Y/N]")) == 'y')
            filesystem::remove(ssi);
        exit(0);
    } else if (inp == "7") {
        if (!filesystem::exists("var/devmode"))
            writetofile(
                "var/devmode",
                "This is a devmode file. You can manually remove this file "
                "to disable dev mode if you don't want to use the program to "
                "disable it for some reason.");
        else
            filesystem::remove("var/devmode");
        startloop("nope");
        exit(0);
    } else if (inp == "8") {
        if (filesystem::exists("var/devmode")) {
            array<string, 6> varlists{
                {"devmode", "zenity", "ds", "asked", "vc", "pe"}};
            for (const string &remove : varlists)
                if (filesystem::exists("var/" + remove))
                    filesystem::remove("var/" + remove);
            writetofile("var/ds", "I forgor why this file exists");
        }
        exit(0);
    }
    if (filesystem::exists("var/devmode")) {
        if (inp == "9") {
            array<string, 6> varlists{
                {"devmode", "zenity", "ds", "asked", "vc", "pe"}};
            for (const string &remove : varlists)
                if (filesystem::exists("var/" + remove))
                    filesystem::remove("var/" + remove);
            writetofile("var/ds", "I forgor why this file exists");
            filesystem::current_path("..");
            if (filesystem::exists("projects"))
                filesystem::remove_all("projects");
            if (filesystem::exists("exports")) {
                filesystem::remove_all("exports");
                filesystem::create_directory("exports");
                writetofile("exports/.temp", "");
            }
            exit(0);
        } else if (inp == "0")
            exit(0);
        else {
            error(inp + " is not an input.");
            inputloop("", aac, aab, realcwd);
        }
    } else {
        error(inp + " is not an input.");
        inputloop("", aac, aab, realcwd);
    }
}

int main(int argc, char *argv[]) // guaranteed 1 argument, which is command used
                                 // to execute executable
{
#ifdef linux
    string dependencies[2] = {"zenity", "unzip"};
    for (const string &dep : dependencies) {
        if (exec(("command -v " + dep).c_str()) == "") {
            cout << dep +
                        ", a necessary dependency, is not installed! Please "
                        "use "
                        "\"sudo apt install " +
                        dep + "\" to install it !"
                 << endl;
            exit(0);
        }
    }
#endif
    startloop("", argc, argv);
}