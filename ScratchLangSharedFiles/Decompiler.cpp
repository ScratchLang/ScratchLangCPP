#include <array>
#include <cmath>
#include <decompiler.h>
#include <filesystem>
#include <fstream>
#include <input.h>
#include <iostream>
#include <regex>
#include <scratchlangfunctions.h>
#include <string>
#include <vector>
#ifdef _WIN32
#include <format>
#endif

using namespace std;

void decompiler(char *argv[], bool args2, string const &realcwd) {
    const bool removejson = false;
    bool dv2dt = false;

    auto dte = [&](string const &arg1) {
        if (dv2dt)
            cout << arg1 << endl;
    };

    if (filesystem::exists("var/devmode"))
        dv2dt = true;
    cout << "\n" + RED + "Decompiler C++ Version 0.0" + NC << endl;
    string dcd = "Stage";
    if (dv2dt) {
        cout << "\n"
             << RED + "Todo List:" + NC << endl
             << "Higher Priorities go first." << endl
             << "----------------------------------------------"
                "------------"
                "---------"
             << endl
             << RED + "* " + NC + "Port decompiler to C++\n"
             << endl
             << "Order of items may change." << endl
             << "----------------------------------------------------------"
                "---------"
             << endl;
    }
    cout << "\nSelect the .sb3 you want to decompile.\n" << endl;
    string sb3file;
    if (!args2) {
        sleep(2000);
        const char *filterPatterns[1] = {"*.sb3"};
#ifdef _WIN32
        filedialog("Choose a Scratch Project",
                   ((string)getenv("USERPROFILE") + "\\Downloads\\").c_str(), 1,
                   filterPatterns, sb3file);
#else
        sb3file = exec("zenity --file-selection --file-filter 'Scratch Project "
                       "(*.sb3) | *.sb3' --file-filter 'All Files (*.*) | *.*' "
                       "--title='Choose a Scratch project'");
        sb3file.pop_back();
#endif
    } else {
        filesystem::path ppath = filesystem::current_path();
        sb3file = argv[2];
        filesystem::current_path(realcwd);
        if (!filesystem::exists(sb3file)) {
            error("File (" + sb3file + ") doesn't exist.");
            exit(0);
        }
        filesystem::current_path(ppath);
    }
    if (sb3file == "") {
        error("Empty path.");
        exit(0);
    }
    cout << "Name of project?" << endl;
    string name;
    getline(cin, name);
    cout << endl;
    if (name == "") {
        error("Project name cannot be empty.");
        exit(0);
    }
    filesystem::current_path("..");
    if (!filesystem::exists("projects"))
        filesystem::create_directory("projects");
    filesystem::current_path("projects");
    if (filesystem::exists(name)) {
        if (tolower(cgetch("Project " + name +
                           " already exists. Replace? [Y/N]")) == 'y')
            filesystem::remove_all(name);
        else
            exit(0);
        cout << endl;
    }
    cout << "Decompiling project...\n" << endl;
    sleep(1000);
    filesystem::create_directory(name);
    filesystem::current_path(name);
    filesystem::path basedir = filesystem::current_path();
    writetofile(".maindir", "Please don't remove this file.");
    writetofile("assets_key.yaml", "");
    cout << "Extracting .sb3...\n" << endl;
    cout << sb3file << endl;
#ifdef _WIN32
    extract_zip(sb3file, ".");
#else
    system(("unzip -q '" + sb3file + "'").c_str());
#endif
    filesystem::create_directories("Stage/assets");
    string jsonfile;
    ifstream file("project.json");
    getline(file, jsonfile);
    file.close();
    int i = 55;
    int kv = -1;
    bool b;
    string character;
    string next = "";
    string varname;
    string varvalue;
    string con;
    string word;
    bool novars;

    auto getcharacter = [&](string_view const &a1) {
        b = false;
        character = jsonfile[i];
        if ("-" + character == a1)
            b = true;
    };

    auto extractdata = [&]() {
        string w2 = "";
        while (true) {
            i++;
            getcharacter("-\"");
            if (b)
                break;
            w2 += character;
        }
        return w2;
    };

    auto nq = [&](int num = 1) {
        for (int k = 0; k < num; k++)
            while (true) {
                i++;
                getcharacter("-\"");
                if (b)
                    break;
            }
    };

    auto start = [&](string_view const &a1 = "") {
        nq(2);
        i += 2;
        getcharacter("-\"");
        if (!b && a1 == "")
            next = "fin";
        else {
            varname = "";
            while (true) {
                i++;
                getcharacter("-\"");
                if (b)
                    break;
                varname += character;
            }
            if (a1 == "") {
                next = varname;
                dte("next: " + next + ", " + varname);
            }
        }
        nq(2);
        i += 2;
        getcharacter("-\"");
        if (!b && a1 == "") {
            writetofile(dcd + "/project.ss1", "\\nscript");
            cout << endl;
        }
        dte(next + "|");
    };

    auto findblock = [&](string const &wordfind) {
        i = (int)jsonfile.find("\"" + wordfind + R"(":{"opcode":)");
    };

    auto writeblock = [&](string const &block) {
        writetofile(dcd + "/project.ss1", block);
        cout << RED + "Added block: " + NC + "\"" + block + "\"" << endl;
    };

    auto addblock = [&](string const &a1, bool dcon = false) {
        string cnext = "";
        start();
        if (a1 == "looks_switchbackdropto") {
            nq(5);
            findblock(extractdata());
            nq(18);
            word = extractdata();
            writeblock("switch backdrop to (\"" + word + "\")");
        } else if (a1 == "looks_switchbackdroptoandwait") {
            nq(5);
            word = extractdata();
            findblock(word);
            nq(18);
            word = extractdata();
            writeblock("switch backdrop to (\"" + word + "\") and wait");
        } else {
            cout << RED + "Unknown block: \"" + a1 + "\" Skipping." + NC
                 << endl;
            writetofile(dcd + "/project.ss1",
                        "DECOMPERR: Unknown block: \"" + a1 + "\"");
        }
    };

    while (true) {
        cout << "Defining variables...\n" << endl;
        int di = i;
        nq();
        word = extractdata();
        i = di;
        if (word != "lists") {
            while (true) {
                nq(2);
                novars = false;
                while (true) {
                    i++;
                    getcharacter("-[");
                    if (b)
                        break;
                    getcharacter("-}");
                    if (b) {
                        novars = true;
                        break;
                    }
                }
                if (!novars) {
                    i++;
                    varname = "";
                    while (true) {
                        i++;
                        getcharacter("-\"");
                        if (b)
                            break;
                        varname += character;
                    }
                    i++;
                    varvalue = "";
                    while (true) {
                        i++;
                        getcharacter("-]");
                        if (b)
                            break;
                        varvalue += character;
                    }
                    if (!filesystem::exists(dcd + "/project.ss1"))
                        writetofile(dcd + "/project.ss1",
                                    "// There should be no empty "
                                    "lines.\nss1\n\\prep");
                    writetofile(dcd + "/project.ss1",
                                "var: " + varname + "=" + varvalue);
                    cout << RED + "Added variable: " + NC + "\"" + varname +
                                "\".\n" + RED + "Value: " + NC + varvalue + "\n"
                         << endl;
                    i++;
                    getcharacter("-}");
                    if (b)
                        break;
                    i -= 2;
                } else {
                    writetofile(
                        dcd + "/project.ss1",
                        "// There should be no empty lines.\nss1\n\\prep");
                    break;
                }
            }
        }
        cout << "Building lists...\n" << endl;
        i = di;
        while (true) {
            word = extractdata();
            if (word == "lists")
                break;
        }
        i++;
        while (true) {
            i += 2;
            getcharacter("-}");
            if (b) {
                novars = true;
                i -= 2;
            } else {
                i -= 2;
                nq(2);
                novars = false;
                while (true) {
                    i++;
                    getcharacter("-[");
                    if (b)
                        break;
                    getcharacter("-}");
                    if (b) {
                        novars = true;
                        break;
                    }
                }
            }
            if (!novars) {
                i++;
                string listname = extractdata();
                i += 3;
                getcharacter("-]");
                if (!b) {
                    vector<string> listcontents;
                    while (true) {
                        getcharacter("-]");
                        if (b)
                            break;
                        if (character == "\"") {
                            varname = extractdata();
                            i++;
                            getcharacter("-]");
                            if (!b) {
                                listcontents.push_back("\"" + varname + "\", ");
                                i++;
                            } else {
                                listcontents.push_back("\"" + varname + "\"");
                                break;
                            }
                            getcharacter("- ");
                            if (b)
                                i++;
                        } else {
                            i--;
                            varname = "";
                            while (true) {
                                i++;
                                getcharacter("-,");
                                if (b)
                                    break;
                                getcharacter("-]");
                                if (b)
                                    break;
                                varname += character;
                            }
                            getcharacter("-]");
                            if (!b)
                                listcontents.push_back("\"" + varname + "\", ");
                            else {
                                listcontents.push_back("\"" + varname + "\"");
                                break;
                            }
                            i++;
                            getcharacter("- ");
                            if (b)
                                i++;
                        }
                    }
                    string list = "";
                    for (string &listing : listcontents) {
                        listing = regex_replace(listing, regex("\n"), "");
                        list += listing;
                    }
                    writetofile(dcd + "/project.ss1",
                                "list: " + listname + "=" + list);
                    cout << RED + "Added list: " + NC + "\"" + listname +
                                "\".\n" + RED + "Contents: " + NC + list
                         << endl;
                } else {
                    writetofile(dcd + "/project.ss1",
                                "list: " + listname + "=,");
                    cout << RED + "Added list: " + NC + "\"" + listname +
                                "\".\n" + RED + "Contents: " + NC + "Nothing."
                         << endl;
                }
            }
            if (novars)
                break;
            i += 2;
            getcharacter("-}");
            if (b)
                break;
        }
        cout << "Loading broadcasts...\n" << endl;
        while (true) {
            string testforbreak = "";
            while (true) {
                i++;
                getcharacter("-\"");
                if (b)
                    break;
                testforbreak += character;
            }
            if (testforbreak == "broadcasts")
                break;
        }
        novars = false;
        i += 3;
        getcharacter("-}");
        if (b) {
            novars = true;
            i -= 2;
        } else {
            i -= 2;
            while (true) {
                i++;
                getcharacter("-\"");
                if (b)
                    break;
                getcharacter("-}");
                if (b) {
                    novars = true;
                    break;
                }
            }
            nq();
        }
        if (!novars) {
            while (true) {
                i++;
                nq();
                varname = "";
                while (true) {
                    i++;
                    getcharacter("-\"");
                    if (b)
                        break;
                    varname += character;
                }
                writetofile(dcd + "/project.ss1", "broadcast: " + varname);
                cout << RED + "Loaded broadcast: " + NC + "\"" + varname +
                            "\"\n"
                     << endl;
                i++;
                getcharacter("-}");
                if (b)
                    break;
                i += 2;
                nq();
            }
        }
        cout << "Making blocks...\n" << endl;
        int k = kv;
        bool done = false;
        while (true) {
            i = k;
            while (true) {
                word = extractdata();
                if (word == "parent") {
                    k = i;
                    i += 2;
                    getcharacter("-\"");
                    if (!b)
                        break;
                }
                if (word == "comments") {
                    done = true;
                    break;
                }
            }
            if (!done) {
                while (true) {
                    i--;
                    getcharacter("-{");
                    if (b)
                        break;
                }
                i++;
                nq(2);
                word = extractdata();
                con = "";
                addblock(word, true);
            } else
                break;
        }
        di = i;
        cout << "\nAdding assets..." << endl;
        string assetsdir = basedir.string() + "/assets_key.yaml";
        writetofile(assetsdir, dcd + ":");
        while (true) {
            word = extractdata();
            if (word == "costumes")
                break;
        }
        while (true) {
            while (true) {
                word = extractdata();
                if (word == "md5ext" || word == "sounds")
                    break;
            }
            if (word == "sounds")
                break;
            nq();
            string assetfile = extractdata();
            writetofile(assetsdir, "  - \"" + assetfile + "\"");
            try {
                filesystem::rename(assetfile, dcd + "/assets/" + assetfile);
                cout << assetfile + " >> " + dcd + "/assets/" + assetfile
                     << endl;
            } catch (filesystem::filesystem_error) {
                // do nothing
            }
            nq(2);
            string a1d = extractdata();
            nq();
            string a2d = extractdata();
            regex target("(:)+|(\\},\\{)+|(\\}\\])+|(,)+");
            a1d = regex_replace(a1d, target, "");
            a2d = regex_replace(a2d, target, "");
            writetofile(assetsdir, "  - \"" + a1d + "\"");
            writetofile(assetsdir, "  - \"" + a2d + "\"");
        }
        if (word != "sounds") {
            while (true) {
                word = extractdata();
                if (word == "sounds")
                    break;
            }
        }
        while (true) {
            while (true) {
                word = extractdata();
                if (word == "md5ext" || word == "volume")
                    break;
            }
            if (word == "volume")
                break;
            nq();
            string assetfile = extractdata();
            writetofile(assetsdir, "  - \"" + assetfile + "\"");
            try {
                filesystem::rename(assetfile, dcd + "/assets/" + assetfile);
                cout << assetfile + " >> " + dcd + "/assets/" + assetfile
                     << endl;
            } catch (filesystem::filesystem_error) {
                // do nothing
            }
        }
        cout << "\nFormatting code...\n" << endl;
        if (!filesystem::exists(dcd + "/project.ss1"))
            writetofile(dcd + "/project.ss1",
                        "// There should be no empty lines.\nss1");
        vector<string> f;
        string readf;
        ifstream infile(dcd + "/project.ss1");
        while (getline(infile, readf))
            f.push_back(readf);
        infile.close();
        string spaces = "";
        i = 0;
        int q = 0;
        auto flen = (int)f.size();
        int proglen = 55;
        int tabsize = 2;
        float per;
        string pstring;
        for (const string &line : f) {
            q++;
            per = static_cast<float>(q) / (float)flen;
            pstring = to_string(round(per * 100));
            pstring = pstring.substr(0, pstring.find("."));
#ifdef _WIN32
            cout << format(
                        "\033[A[{}{}] {}%",
                        string((int)round((float)proglen * per), '#'),
                        string(proglen - (int)round((float)proglen * per), ' '),
                        pstring)
                 << endl;
#else
            cout << "\033[A[" + string((int)round((float)proglen * per), '#') +
                        string(proglen - (int)round((float)proglen * per),
                               ' ') +
                        "] " + pstring + "%"
                 << endl;
#endif
            if (line.find('}') != string::npos &&
                line.find('{') != string::npos) {
                i--;
                spaces = string(tabsize, ' ') * i;
                i++;
            } else if (line.find('{') != string::npos) {
                spaces = string(tabsize, ' ') * i;
                i++;
            } else if (line.find('}') != string::npos) {
                i--;
                spaces = string(tabsize, ' ') * i;
            } else
                spaces = string(tabsize, ' ') * i;
            if (line != "")
                writetofile(dcd + "/a.txt", spaces + line);
        }
        filesystem::remove(dcd + "/project.ss1");
        filesystem::rename(dcd + "/a.txt", dcd + "/project.ss1");
        i = di;
        while (true) {
            word = extractdata();
            if (word == "isStage" || word == "agent")
                break;
        }
        if (word == "agent")
            break;
        kv = i;
        nq(3);
        dcd = extractdata();
        dcd = regex_replace(dcd, regex(" "), "-");
        array<string, 9> replacings = {"/",  R"(\\)", "<",   ">",  ":",
                                       "\"", "|",     "\\?", "\\*"};
        for (const string &replacing : replacings)
            dcd = regex_replace(dcd, regex(replacing), "");
        filesystem::create_directories(dcd + "/assets");
        nq(2);
        i += 2;
        cout << "Now entering: " + dcd << endl;
    }
    filesystem::current_path("..");
    string dir = filesystem::current_path().string();
    if (removejson)
        filesystem::remove(dir + "/" + name + "/" + "project.json");
#ifdef _WIN32
    cout << RED + "Your project can be found in " + dir + "\\" + name + "." +
                NC + "\nOpen in ScratchLang editor? [Y/N]"
         << endl;
#else
    cout << RED + "Your project can be found in " + dir + "/" + name + "." +
                NC + "\nOpen in ScratchLang editor? [Y/N]"
         << endl;
#endif
    cout << "I'm sorry, but the C++ version of the editor is not available "
            "yet. You can still use the Python version though."
         << endl;
    exit(0);
}