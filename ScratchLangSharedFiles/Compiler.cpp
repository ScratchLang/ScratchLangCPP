#include <algorithm>
#include <compiler.h>
#include <filesystem>
#include <iostream>
#include <map>
#include <random>
#include <regex>
#include <scratchlangfunctions.h>
#include <sstream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#ifdef _WIN32
#else
#include <string.h>
#endif

using namespace std;

void compiler(char *argv[]) {
    auto generaterandomid = [](int length) {
        string letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUV"
                         "WXYZ1234567890`-=[];./~!@#$%^&*()_+{}|:<>?";
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<mt19937::result_type> distr(1,
                                                             letters.length());
        string result_string;
        result_string.reserve(length);
        for (int i = 0; i < length; i++) {
            char let = '"';
            result_string += letters[distr(gen)];
        }
        return result_string;
    };
    auto isnumber = [](const string &s) {
        return !s.empty() && find_if(s.begin(), s.end(), [](unsigned char c) {
                                 return !isdigit(c);
                             }) == s.end();
    };

    cout << "\n"
         << endl
         << RED + "Compiler C++ Version 1.0" + NC + "\n"
         << endl
         << "Remember, the C++ Compiler isn't finished yet.\n"
         << endl
         << "Select your project folder.\n"
         << endl;
    sleep(2000);
    string fold = "";
    if (sizeof(argv) / sizeof(argv[0]) > 1)
        fold = argv[1];
    else {
#ifdef _WIN32
        try {
            fold = tinyfd_selectFolderDialog(
                "Choose a project.",
                (filesystem::current_path().parent_path().string() +
                 "\\projects\\")
                    .c_str());
        } catch (...) {
            error("Empty path.");
            exit(0);
        }
#else
        fold = exec("zenity --file-selection --directory");
        fold.erase(remove(fold.begin(), fold.end(), '\n'), fold.cend());
#endif
    }
    if (fold != "") {
        filesystem::current_path(fold);
        if (!filesystem::exists(".maindir")) {
            cout << "Not a ScratchLang project (" + fold +
                        "), or .maindir file was deleted."
                 << endl;
            exit(0);
        }
    } else {
        error("Empty path.");
        exit(0);
    }
    vector<string> realdirs;
    vector<string> namedirs;
    for (const auto &fff : filesystem::directory_iterator(".")) {
        if (filesystem::is_directory(filesystem::canonical(fff.path()))) {
            realdirs.push_back(filesystem::canonical(fff.path()).string());
            namedirs.push_back(fff.path().string().erase(0, 2));
        }
    }
    wstring ws(1, filesystem::path::preferred_separator);
    string rdelim(ws.begin(), ws.end());
    realdirs.erase(
        remove(realdirs.begin(), realdirs.end(),
               filesystem::current_path().string() + rdelim + "Stage"),
        realdirs.end());
    realdirs.insert(realdirs.begin(),
                    filesystem::current_path().string() + rdelim + "Stage");
    namedirs.erase(remove(namedirs.begin(), namedirs.end(), "Stage"),
                   namedirs.end());
    namedirs.insert(namedirs.begin(), "Stage");
    string json = "{\"targets\":[";
    string indir = filesystem::current_path().string();
    int k = -1;
    for (string &dirs : realdirs) {
        k++;
        filesystem::current_path(dirs);
        dirs = namedirs[k];
        string assetfolder = filesystem::current_path().string() + "/assets";
        if (json.ends_with("],\"monitors\":[],\"extensions\":[],"
                           "\"meta\":{\"semver\":\"3.0.0\",\"vm\":\"0.2.0\","
                           "\"agent\":\"\"}}")) {
            json.erase(json.find("],\"monitors\":[],\"extensions\":[],"
                                 "\"meta\":{\"semver\":\"3.0.0\",\"vm\":\"0.2."
                                 "0\",\"agent\":\"\"}}"),
                       json.length());
            json += ',';
        }
        json +=
            (string) "{\"isStage\":" + ((dirs == "Stage") ? "true" : "false") +
            ",\"name\":\"" + dirs + "\",\"variables\":{";
        ifstream f("project.ss1");
        string line;
        vector<string> filelines;
        while (getline(f, line))
            filelines.push_back(line);
        f.close();
        int i = -1;
        string ssver = "";
        map<string, string> varidmap;
        map<string, string> listidmap;
        map<string, string> broadcastidmap;
        while (true) {
            i++;
            if (i >= filelines.size() - 1 || filelines[i].starts_with("ss")) {
                ssver = filelines[i];
                break;
            }
        }
        while (true) {
            i++;
            if (i >= filelines.size() - 1 || filelines[i].starts_with("\\prep"))
                break;
        }
        string variableid = "";
        string vardata;
        for (int j = i + 1; j < filelines.size(); j++) {
            if (filelines[j].starts_with("\\nscript"))
                break;
            if (filelines[j].starts_with("var: ")) {
                line = filelines[j];
                line.erase(0, 5);
                string vardata[2] = {"", ""};
                int cc;
                for (int count = 0; count < line.length(); count++) {
                    if (line[count] == '=') {
                        cc = count;
                        break;
                    }
                    vardata[0] += line[count];
                }
                for (int count = cc + 1; count < line.length(); count++) {
                    cc++;
                    vardata[1] += line[cc];
                }
                variableid = generaterandomid(20);
                vardata[1] = regex_replace(vardata[1], regex("\""), "");
                if (isnumber(vardata[1]))
                    json += "\"" + variableid + "\":[\"" + vardata[0] + "\"," +
                            vardata[1] + "],";
                else
                    json += "\"" + variableid + "\":[\"" + vardata[0] +
                            "\",\"" + vardata[1] + "\"],";
                varidmap.insert({vardata[0], variableid});
            }
        }
        if (json.ends_with(","))
            json.pop_back();
        json += "},\"lists\":{";
        for (int j = i + 1; j < filelines.size(); j++) {
            if (filelines[j].starts_with("\\nscript"))
                break;
            if (filelines[j].starts_with("list: ")) {
                line = filelines[j];
                line.erase(0, 6);
                string vardata[2] = {"", ""};
                int cc;
                for (int count = 0; count < line.length(); count++) {
                    if (line[count] == '=') {
                        cc = count;
                        break;
                    }
                    vardata[0] += line[count];
                }
                for (int count = cc + 1; count < line.length(); count++) {
                    cc++;
                    vardata[1] += line[cc];
                }
                variableid = generaterandomid(20);
                json += "\"" + variableid + "\":[\"" + vardata[0] + "\",[";
                vector<string> listdata;
                int xd = -1;
                while (true) {
                    string item = "";
                    while (true) {
                        xd++;
                        if (xd > vardata[1].length() - 1)
                            break;
                        if (vardata[1][xd] == '"')
                            break;
                    }
                    while (true) {
                        xd++;
                        if (xd > vardata[1].length() - 1)
                            break;
                        if (vardata[1][xd] == '"')
                            break;
                        item += vardata[1][xd];
                    }
                    if (xd > vardata[1].length() - 1)
                        break;
                    listdata.push_back(item);
                }
                if ((listdata.size() == 2 && listdata[0] + listdata[1] == "") ||
                    listdata.size() == 0)
                    json += "]],";
                else {
                    for (const string &x : listdata)
                        json += "\"" + x + "\", ";
                    json.pop_back();
                    json += "]],";
                }
                listidmap.insert({vardata[0], variableid});
            }
        }
        if (json.ends_with(","))
            json.pop_back();
        json += "},\"broadcasts\":{";
        for (int j = i + 1; j < filelines.size(); j++) {
            if (filelines[j].starts_with("\\nscript"))
                break;
            if (filelines[j].starts_with("broadcast: ")) {
                line = filelines[j];
                line.erase(0, 11);
                string vardata[2] = {"", ""};
                istringstream stream(line);
                string token;
                int count = -1;
                while (getline(stream, token, '=')) {
                    count++;
                    vardata[count] = token;
                }
                variableid = generaterandomid(20);
                json += "\"" + variableid + "\":\"" + vardata[0] + "\",";
                broadcastidmap.insert({vardata[0], variableid});
            }
        }
        if (json.ends_with(","))
            json.pop_back();
        json += "},\"blocks\":{},\"comments\":{},\"currentCostume\":0,"
                "\"costumes\":[";
        YAML::Node assetskey = YAML::LoadFile(indir + "/assets_key.yaml");
        YAML::Node asset = assetskey[dirs];
        int cosId = 0;
        int n = -1;
        string item;
        while (true) {
            n++;
            if (n > asset.size() - 1)
                break;
            item = asset[n].as<string>();
            string fname =
                ((filesystem::path)item).replace_extension().string();
            string fext =
                ((filesystem::path)item).extension().string().erase(0, 1);
            if (fext == "svg" || fext == "png" || fext == "jpg") {
                cosId++;
                n++;
                string rotCX = asset[n].as<string>();
                n++;
                string rotCY = asset[n].as<string>();
                json += "{\"name\":\"" + to_string(cosId) +
                        "\",\"dataFormat\":\"" + fext + "\",\"assetId\":\"" +
                        fname + "\",\"md5ext\":\"" + item +
                        "\",\"rotationCenterX\":" + rotCX +
                        ",\"rotationCenterY\":" + rotCY + "},";
            }
        }
        if (json.ends_with(","))
            json.pop_back();
        json += "],\"sounds\":[";
        cosId = 0;
        for (const auto &item : asset) {
            string fname = ((filesystem::path)item.as<string>())
                               .replace_extension()
                               .string();
            string fext = ((filesystem::path)item.as<string>())
                              .extension()
                              .string()
                              .erase(0, 1);
            if (fext == "wav" || fext == "mp3") {
                cosId++;
                json += "{\"name\":\"" + to_string(cosId) +
                        "\",\"assetId\":\"" + fname + "\",\"dataFormat\":\"" +
                        fext +
                        "\",\"rate\":48000,\"sampleCount\":"
                        "99999999,\"md5ext\":\"" +
                        item.as<string>() + "\"},";
            }
        }
        if (json.ends_with(","))
            json.pop_back();
        if (dirs == "Stage")
            json += "],\"volume\":100,\"layerOrder\":" + to_string(k) +
                    ",\"tempo\":"
                    "60,"
                    "\"videoTransparency\":50,\"videoState\":\"on\","
                    "\"textToSpeechLanguage\":null}],\"monitors\":[],"
                    "\"extensions\":[],"
                    "\"meta\":{\"semver\":\"3.0.0\",\"vm\":\"0.2.0\",\"agent\":"
                    "\"\"}}";
        else
            json += "],\"volume\":100,\"layerOrder\":" + to_string(k) +
                    ",\"visible\":true,\"x\":"
                    "0,\"y\":0,\"size\":100,\"direction\":90,\"draggable\":"
                    "false,\"rotationStyle\":\"all around\"}],\"monitors\":[],"
                    "\"extensions\":[],"
                    "\"meta\":{\"semver\":\"3.0.0\",\"vm\":\"0.2.0\",\"agent\":"
                    "\"\"}}";
        filesystem::path ppath = filesystem::current_path();
        filesystem::current_path(
            ((filesystem::path)getexecwd()).parent_path().string() +
            "/data/exports");
        if (!filesystem::exists("a"))
            filesystem::create_directory("a");
        filesystem::current_path("a");
        for (const filesystem::directory_entry &iter :
             filesystem::directory_iterator(assetfolder))
            filesystem::copy(iter.path(), filesystem::current_path());
    }
    writetofile("project.json", json);
    filesystem::current_path("..");
#ifdef _WIN32
    mz_zip_archive zip_archive = {0};
    mz_zip_writer_init_file(&zip_archive, "project.sb3", 0);
    add_to_zip(zip_archive, "a");
    mz_zip_writer_finalize_archive(&zip_archive);
    mz_zip_writer_end(&zip_archive);
#else
    system("tar -cf project.sb3 a");
#endif
    filesystem::remove_all("a");
    cout << json << endl;
}