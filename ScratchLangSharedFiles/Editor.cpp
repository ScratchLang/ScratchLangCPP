#include <algorithm>
#include <cmath>
#include <filesystem>
#include <input.h>
#include <iostream>
#include <map>
#include <regex>
#include <scratchlangfunctions.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#else
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

using namespace std;

void setcursorvisibility(bool show) {
#ifdef _WIN32
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize = 100;
    cci.bVisible = show;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
#else
    cout << (show ? "\033[?25h" : "\033[?25l");
#endif
}

void editor() {
    setcursorvisibility(false);
    string MNC = NC;
    bool inEditor = true;
    bool cursorBlink = false;
    filesystem::current_path(
        ((filesystem::path)getexecwd()).parent_path().string() +
        "/data/mainscripts");
    string cwd = filesystem::current_path().string();

    auto getdimensions = [&](int &h, int &w) {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
        struct winsize ww;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &ww);
        w = ww.ws_col;
        h = ww.ws_row;
#endif
    };
    int terminalheight, terminalwidth;
    getdimensions(terminalheight, terminalwidth);
    cout << "\nSelect your project folder.\n" << endl;
    sleep(2000);
    string folder;
#ifdef _WIN32
    try {
        folder = tinyfd_selectFolderDialog(
            "Choose a project.",
            (filesystem::current_path().parent_path().string() + "\\projects\\")
                .c_str());
    } catch (...) {
        error("Folder not selected.");
        exit(0);
    }
#else
    folder = exec("zenity --file-selection --directory");
    folder.erase(remove(folder.begin(), folder.end(), '\n'), folder.cend());
#endif
    if (folder == "") {
        error("Folder not selected.");
        exit(0);
    }
    string ppath = filesystem::current_path().string();
    filesystem::current_path(folder);
    folder = filesystem::current_path().string() + "/Stage";
    string realfolderpath = ((filesystem::path)folder).parent_path().string();
    filesystem::current_path(ppath);
    if (!filesystem::exists(realfolderpath + "/.maindir")) {
        error("Not a ScratchScript project (" + realfolderpath +
              "), or .maindir file was deleted.");
        exit(0);
    }
    filesystem::current_path(realfolderpath);
    vector<string> editorLines;
    cout << "Loading " + realfolderpath + "...\n" << endl;
    filesystem::current_path("Stage");
    ifstream ss1("project.ss1");
    string f;
    vector<string> fileOpened;
    while (getline(ss1, f))
        fileOpened.push_back(f);
    ss1.close();
    int fileOpenedLength = fileOpened.size();
    int editorLinesCount;
    int proglen = 55;
    int q = 0;
    float percent;
    string pstring;
    for (const string &line : fileOpened) {
        q++;
        percent = static_cast<float>(q) / (float)fileOpenedLength;
        pstring = to_string(round(percent * 100));
        pstring = pstring.substr(0, pstring.find("."));
        cout << "\033[A[" + string((int)round((float)proglen * percent), '#') +
                    string(proglen - (int)round((float)proglen * percent),
                           ' ') +
                    "] " + pstring + "%"
             << endl;
        editorLines.push_back(line + ' ');
    }
    editorLinesCount = editorLines.size();
    // add yaml settings later
    int tabSize = 2;
    bool syntaxHighlightingEnabled = true;
    bool showCWD = true;
    string themeAnsi = "\033[48;2;35;37;41m";
    MNC += themeAnsi;
    int editorCurrentLine, editorChar, realLine;
    cout << "\nBuilding syntax highlighting...\n" << endl;
    q = 0;
    map<string, string> colors{
        {"c", "\033[0m" + themeAnsi},    {"p", "\033[48;5;10m"},
        {"n", "\033[48;5;10m"},          {"0", "\033[37m"},
        {"1", "\033[38;2;153;102;255m"}, {"2", "\033[38;2;255;140;26m"},
        {"3", "\033[38;2;255;102;26m"},  {"4", "\033[38;2;255;191;0m"},
        {"5", "\033[38;2;207;99;207m"},  {"6", "\033[38;2;255;171;25m"},
        {"7", "\033[38;2;92;177;214m"},  {"8", "\033[38;2;89;192;89m"},
        {"9", "\033[38;2;15;189;140m"},  {"10", "\033[38;2;76;151;255m"}};
    map<string, string> backgroundColors{
        {"c", ""}, {"p", "\033[1;90m"}, {"n", "\033[1.90m"}, {"0", ""},
        {"1", ""}, {"2", ""},           {"3", ""},           {"4", ""},
        {"5", ""}, {"6", ""},           {"7", ""},           {"8", ""},
        {"9", ""}, {"10", ""}};
    map<string, string> parenthesisColors{{"0", ""},
                                          {"1", "\033[31m"},
                                          {"2", "\033[38;5;202m"},
                                          {"3", "\033[33m"},
                                          {"4", "\033[32m"},
                                          {"5", "\033[34m"},
                                          {"6", "\033[38;5;135m"},
                                          {"7", "\033[35m"},
                                          {"8", "\033[38;5;206m"}};
    vector<string> looks = {
        "switch backdrop to (",
        "next backdrop",
        "change [c",
        "change [f",
        "change [w",
        "change [pix",
        "change [m",
        "change [b",
        "change [g",
        "clear graphic effects",
        "(backdrop [",
        "set [c",
        "set [f",
        "set [w",
        "set [pix",
        "set [m",
        "set [b",
        "set [g",
        "say (", // sprite blocks
        "think (",
        "switch costume to (",
        "next costume",
        "change size by (",
        "set size to (",
        "show",
        "hide",
        "go to [f",
        "go to [b",
        "go [f",
        "go [b",
        "(size)",
        "(costume [",
    };
    vector<string> looksFindType = {
        "le", "eq", "le", "le", "le", "le", "le", "le", "le", "eq", "le",
        "le", "le", "le", "le", "le", "le", "le", "le", "le", "le", "eq",
        "le", "le", "eq", "eq", "le", "le", "le", "le", "eq", "le",
    };
    vector<string> dataVar = {
        "var:", "] to (", "] by (", "show variable [", "hide variable [",
    };
    vector<string> dataVarFindType = {
        "le", "in", "in", "le", "le",
    };
    vector<string> dataList = {
        "list:",        ") to [",       ") by [",         "delete all of [",
        "delete (",     "insert (",     "replace item (", "(item (",
        "(item # of (", "(length of [", "] contains (",   "show list [",
        "hide list [",
    };
    vector<string> dataListFindType = {
        "le", "in", "in", "le", "le", "le", "le",
        "le", "le", "le", "in", "le", "le",
    };
    vector<string> events = {
        "broadcast [",
        "broadcast:",
        "when I receive [",
        "when [",
        "when flag clicked",
        "when backdrop switches to [",
        "when this sprite clicked",
    };
    vector<string> eventsFindType = {"le", "le", "le", "le", "eq", "le", "eq"};
    vector<string> sounds = {
        "play sound (",       "start sound (",   "change [pit",
        "change [pan",        "set [pit",        "set [pan",
        "stop all sounds",    "(volume)",        "clear sound effects",
        "change volume by (", "set volume to (",
    };
    vector<string> soundsFindType = {
        "le", "le", "le", "le", "le", "le", "eq", "eq", "eq", "le", "le",
    };
    vector<string> control = {
        "wait (",
        "wait until <",
        "repeat (",
        "repeat until <",
        "forever {",
        "if <",
        "} else {",
        "while <",
        "create a clone of (",
        "for [",
        "stop [",
        "when I start as a clone",
        "delete this clone",
    };
    vector<string> controlFindType = {
        "le", "le", "le", "le", "le", "le", "eq",
        "le", "le", "le", "le", "eq", "eq",
    };
    vector<string> sensing = {
        "ask (",
        "(answer)",
        "<key (",
        "<mouse down?>",
        "(mouse x)",
        "(mouse y)",
        "(loudness)",
        "(timer)",
        "reset timer",
        "([",
        "(current [",
        "(days since 2000)",
        "(username)",
        "<touching (",
        "<touching color (",
        "<color (",
        "(distance to (",
        "set drag mode [",
    };
    vector<string> sensingFindType = {
        "le", "eq", "le", "eq", "eq", "eq", "eq", "eq", "eq",
        "le", "le", "eq", "eq", "le", "le", "le", "le", "le",
    };
    vector<string> operators = {
        ") + (",     ") - (",        ") / (",          ") * (",
        ") > (",     ") < (",        "(pick random (", ") = (",
        "> and <",   "> or <",       "<not <",         "(join (",
        "(letter (", "(length of (", ") contains (",   ") mod (",
        "(round (",
    };
    vector<string> operatorsFindType = {
        "in", "in", "in", "in", "in", "in", "le", "in", "in",
        "in", "le", "le", "le", "le", "in", "in", "le",
    };
    vector<string> motion = {
        "move (",
        "turn cw (",
        "turn ccw (",
        "go to (",
        "go to x: (",
        "glide (",
        "point in direction (",
        "point towards (",
        "change x by (",
        "set x to (",
        "change y by (",
        "set y to (",
        "if on edge, bounce",
        "set rotation style [",
        "(x position)",
        "(y position)",
        "(direction)",
    };
    vector<string> motionFindType = {
        "le", "le", "le", "le", "le", "le", "le", "le", "le",
        "le", "le", "le", "eq", "le", "eq", "eq", "eq",
    };
    vector<string> pen = {
        "erase all",          "stamp",
        "pen down",           "pen up",
        "set pen color to (", "change pen (",
        "set pen (",          "change pen size by (",
        "set pen size to (",
    };
    vector<string> penFindType = {"eq", "eq", "eq", "eq", "le",
                                  "le", "le", "le", "le"};
    vector<string> startParenthesis = {"(", "[", "{", "<"};
    vector<string> endParenthesis = {")", "]", "}", ">"};
    string excludes[] = {"46;1", "38;5;8", "0", "37", "35", "7", "1"};
    int bracketCount = 0;
    vector<string> shabang = {
        "//!looks",   "//!var",     "//!list",      "//!events", "//!sound",
        "//!control", "//!sensing", "//!operators", "//!pen",    "//!motion",
    };
    string syntaxType;
    string syntaxLine;
    map<string, vector<string>> blocks{
        {"8", operators}, {"1", looks},  {"2", dataVar}, {"3", dataList},
        {"4", events},    {"5", sounds}, {"7", sensing}, {"6", control},
        {"9", pen},       {"10", motion}};
    map<string, vector<string>> blocksFindType{
        {"8", operatorsFindType}, {"1", looksFindType},
        {"2", dataVarFindType},   {"3", dataListFindType},
        {"4", eventsFindType},    {"5", soundsFindType},
        {"7", sensingFindType},   {"6", controlFindType},
        {"9", penFindType},       {"10", motionFindType}};
    int parenthesisCount;
    bool shift = false;
    bool ctrl = false;

    auto DTLoop = [&](string line, string type) {
        int qt = -1;
        for (const string &i : blocks[type]) {
            qt++;
            string tofind = blocksFindType[type][qt];
            if (((tofind == "le" &&
                  regex_replace(line, regex("^ +"), "").starts_with(i)) ||
                 tofind == "eq" &&
                     regex_replace(line, regex("^ +| +$|() +"), "$1") == i) ||
                tofind == "in" &&
                    regex_replace(line, regex("^ +"), "").find(i) !=
                        string::npos)
                syntaxType = type;
        }
    };

    auto DetermineType = [&](string line) {
        syntaxType = "0";
        int syntaxTypeId = 0;
        DTLoop(line, "8");
        DTLoop(line, "1");
        DTLoop(line, "2");
        DTLoop(line, "3");
        DTLoop(line, "4");
        DTLoop(line, "5");
        DTLoop(line, "7");
        DTLoop(line, "6");
        DTLoop(line, "9");
        DTLoop(line, "10");
        for (const string &i : shabang) {
            syntaxTypeId++;
            if (regex_replace(line, regex("^ +"), "").find(i) != string::npos)
                syntaxType = to_string(syntaxTypeId);
        }
        if (regex_replace(line, regex("^ +"), "").find("\\nscript") !=
            string::npos)
            syntaxType = "n";
        if (regex_replace(line, regex("^ +"), "").find("\\prep") !=
            string::npos)
            syntaxType = "p";
    };

    auto AddSyntax = [&](string line, bool progressBarEnabled = true) {
        syntaxLine = line;
        q++;
        percent = static_cast<float>(q) / (float)fileOpenedLength;
        if (progressBarEnabled) {
            pstring = to_string(round(percent * 100));
            pstring = pstring.substr(0, pstring.find("."));
            cout << "\033[A[" +
                        string((int)round((float)proglen * percent), '#') +
                        string(proglen - (int)round((float)proglen * percent),
                               ' ') +
                        "] " + pstring + "%"
                 << endl;
        }
        DetermineType(line);
        int i = -1;
        string buildedLine = "";
        parenthesisCount = bracketCount;
        char character;
        while (true) {
            i++;
            if (i < syntaxLine.length())
                character = syntaxLine[i];
            else
                break;
            buildedLine += character;
            if (find(startParenthesis.begin(), startParenthesis.end(),
                     string(character, 1)) != startParenthesis.end()) {
                if (character != '{') {
                    if (character == '<') {
                        if (!(syntaxLine[i - 1] == ' ' &&
                              syntaxLine[i + 1] == ' ')) {
                            parenthesisCount++;
                            int c = 0;
                            for (int z = 0; z < parenthesisCount; z++) {
                                c++;
                                if (c == 9)
                                    c = 1;
                            }
                            buildedLine.pop_back();
                            buildedLine += parenthesisColors[to_string(c)] +
                                           character + colors[syntaxType];
                        }
                    } else {
                        parenthesisCount++;
                        int c = 0;
                        for (int z = 0; z < parenthesisCount; z++) {
                            c++;
                            if (c == 9)
                                c = 1;
                        }
                        buildedLine.pop_back();
                        buildedLine += parenthesisColors[to_string(c)] +
                                       character + colors[syntaxType];
                    }
                } else {
                    bracketCount++;
                    int c = 0;
                    for (int z = 0; z < bracketCount; z++) {
                        c++;
                        if (c == 9)
                            c = 1;
                    }
                    buildedLine.pop_back();
                    buildedLine += parenthesisColors[to_string(c)] + character +
                                   colors[syntaxType];
                }
            } else if (find(endParenthesis.begin(), endParenthesis.end(),
                            string(character, 1)) != endParenthesis.end()) {
                if (character != '}') {
                    if (character == '<') {
                        if (!(syntaxLine[i - 1] == ' ' &&
                              syntaxLine[i + 1] == ' ') &&
                            parenthesisCount > 0) {
                            int c = 0;
                            for (int z = 0; z < bracketCount; z++) {
                                c++;
                                if (c == 9)
                                    c = 1;
                            }
                            buildedLine.pop_back();
                            buildedLine += parenthesisColors[to_string(c)] +
                                           character + colors[syntaxType];
                            parenthesisCount--;
                        }
                    } else if (parenthesisCount > 0) {
                        int c = 0;
                        for (int z = 0; z < bracketCount; z++) {
                            c++;
                            if (c == 9)
                                c = 1;
                        }
                        buildedLine.pop_back();
                        buildedLine += parenthesisColors[to_string(c)] +
                                       character + colors[syntaxType];
                        parenthesisCount--;
                    }
                } else if (bracketCount > 0) {
                    int c = 0;
                    for (int z = 0; z < bracketCount; z++) {
                        c++;
                        if (c == 9)
                            c = 1;
                    }
                    buildedLine.pop_back();
                    buildedLine += parenthesisColors[to_string(c)] + character +
                                   colors[syntaxType];
                    bracketCount--;
                }
            }
            if (character == '"') {
                buildedLine.pop_back();
                buildedLine += "\033[38;5;34m\"";
                while (true) {
                    i++;
                    if (i < syntaxLine.length())
                        character = syntaxLine[i];
                    else
                        break;
                    buildedLine += character;
                    if (character == '"') {
                        buildedLine += colors[syntaxType];
                        break;
                    }
                }
            }
            if (character == '\'') {
                buildedLine.pop_back();
                buildedLine += "\033[38;5;34m'";
                while (true) {
                    i++;
                    if (i < syntaxLine.length())
                        character = syntaxLine[i];
                    else
                        break;
                    buildedLine += character;
                    if (character == '\'') {
                        buildedLine += colors[syntaxType];
                        break;
                    }
                }
            }
            if (character == '/') {
                i--;
                if (i < syntaxLine.length()) {
                    if (syntaxLine[i] == '/') {
                        i++;
                        buildedLine.erase(buildedLine.length() - 2);
                        buildedLine += "\033[38;5;8m//";
                        while (true) {
                            i++;
                            if (i < syntaxLine.length())
                                character = syntaxLine[i];
                            else
                                break;
                            buildedLine += character;
                        }
                        break;
                    } else
                        i++;
                } else
                    i++;
            }
        }
        syntaxLine = buildedLine;
        string syntaxBuild = backgroundColors[syntaxType] + colors[syntaxType] +
                             syntaxLine + "\033[0m" + themeAnsi;
        if (syntaxBuild == "")
            syntaxBuild = line;
        return syntaxBuild;
    };

    bool lineWrapWarning = false;
    vector<string> editorLinesWithSyntax;
    for (const string &line : editorLines) {
        if (line.length() > terminalwidth)
            lineWrapWarning = true;
        editorLinesWithSyntax.push_back(AddSyntax(line));
    }
    if (lineWrapWarning) {
        cout << "\nWARNING: Line wrap may occur and will make the editor "
                "glitch out. Instead of using this, you should use something "
                "like Notepad or VSCode to edit the ScratchScript file, as "
                "it's much better than this editor."
             << endl;
        sleep(2000);
    }
    editorCurrentLine = 1;
    editorChar = editorLines[0].length();
    realLine = 1;
    bool quoteComplete = false;
    bool singleQuoteComplete = false;
    int parenthesisComplete = 0;
    string taskbar;
    string taskbarMessage = "";
    string state = "edit";
    string validKeys[22] = {"F1",        "F2",        "F3",         "F4",
                            "F5",        "F6",        "F7",         "F8",
                            "F9",        "F10",       "F11",        "F12",
                            "Enter",     "Tab",       "Backspace",  "UpArrow",
                            "DownArrow", "LeftArrow", "RightArrow", "Delete",
                            "PageUp",    "PageDown"};

    auto editorprint = [&](int line) {
        int getlinecount = to_string(editorLinesCount).length();
        string cwdstring;
        string editorbuffer;
        if (showCWD) {
            cwdstring = "\033[48;2;56;113;228m\033[35;1m Current Working "
                        "Directory: " +
                        folder + "\\project.ss1 ";
            if ((" Current Working Directory: " + folder + "\\project.ss1")
                    .length() > terminalwidth) {
                cwdstring =
                    "\033[48;2;56;113;228m\033[35;1m" +
                    (" Current Working Directory: " + folder + "\\project.ss1")
                        .erase(terminalwidth - 5, string::npos) +
                    "...";
                editorbuffer = "\033[48;2;56;113;228m" +
                               string(terminalwidth, ' ') + "\n\033[A" +
                               cwdstring + "\n" + themeAnsi;
            } else
                editorbuffer =
                    cwdstring +
                    string((terminalwidth - (" Current Working Directory: " +
                                             folder + "\\project.ss1")
                                                .length()) -
                               1,
                           ' ') +
                    "\033[0m\n" + themeAnsi;
        } else {
            cwdstring = "\033[48;2;56;113;228m\033[35;1m";
            editorbuffer = cwdstring + string(terminalwidth, ' ') +
                           "\033[0m\n" + themeAnsi;
        }
        q = realLine - 1;
        string editorbufferline = "";
        bool thist = false;
        for (int i = 0; i < line - 2; i++) {
            q++;
            string filler = "";
            if (thist)
                editorbufferline =
                    "\033[38;5;8m ~" +
                    string(to_string(editorLinesCount).length() + 2, ' ') +
                    "\033[1;38;5;8m|\033[0m" + themeAnsi +
                    string(terminalwidth -
                               (to_string(editorLinesCount).length() + 5),
                           ' ') +
                    "\n";
            else {
                filler = string(terminalwidth -
                                    (editorLines[q - 1].length() +
                                     to_string(editorLinesCount).length() + 5),
                                ' ');
                editorbufferline =
                    "\033[38;5;8m" +
                    string(getlinecount - to_string(q).length(), ' ') +
                    ((editorCurrentLine == q) ? "\033[93m" : "") +
                    to_string(q) + "\033[1;38;5;8m    |\033[0m" + themeAnsi +
                    editorLinesWithSyntax[q - 1] + filler + "\n";
                if (editorCurrentLine == q && cursorBlink) {
                    string editorbufferbuffer = editorLines[q - 1];
                    DetermineType(editorbufferbuffer);
                    string findd = colors[syntaxType];
                    parenthesisCount = 0;
                    bracketCount = 0;
                    int j = -1;
                    for (int k = 0; k < editorbufferbuffer.length(); k++) {
                        bool quote = false;
                        bool comment = false;
                        j++;
                        if (editorChar == j + 1)
                            findd += "\033[46;1m";
                        if (j > editorbufferbuffer.length() - 1) {
                            j++;
                            break;
                        } else {
                            switch (editorbufferbuffer[j]) {
                            case '"':
                                findd += "\033[38;5;34m\"";
                                quote = true;
                                break;
                            case '\'':
                                findd += "\033[38;5;34m'";
                                quote = true;
                                break;
                            case '/':
                                if (j > 0) {
                                    j--;
                                    if (editorbufferbuffer[j] == '/') {
                                        j++;
                                        if (editorChar == j)
                                            findd += "\033[46;1m\033[38;5;8m/"
                                                     "\033[0m\033[38;5;8m" +
                                                     themeAnsi + "/";
                                        else if (editorChar == j + 1)
                                            findd += "\033[0m" + themeAnsi +
                                                     "\033[38;5;8m/\033[46;1m/"
                                                     "\033[0m";
                                        else
                                            findd += "\033[38;5;8m//";
                                        comment = true;
                                    } else
                                        j++;
                                }
                                break;
                            default:
                                findd += editorbufferbuffer[j];
                                break;
                            }
                        }
                        char character = editorbufferbuffer[j];
                        if (editorChar == j + 1)
                            findd += "\033[0m" + themeAnsi + colors[syntaxType];
                        if (find(startParenthesis.begin(),
                                 startParenthesis.end(),
                                 string(character, 1)) !=
                            startParenthesis.end()) {
                            if (character != '{') {
                                if (character == '<') {
                                    if (!(editorbufferbuffer[j - 1] == ' ' ||
                                          editorbufferbuffer[j + 1] == ' ')) {
                                        parenthesisCount++;
                                        int c = 0;
                                        for (int z = 0; z < parenthesisCount;
                                             z++) {
                                            c++;
                                            if (c == 9)
                                                c = 1;
                                        }
                                        findd.pop_back();
                                        findd +=
                                            parenthesisColors[to_string(c)] +
                                            ((editorChar != j + 1)
                                                 ? string(character, 1)
                                                 : "") +
                                            colors[syntaxType];
                                    } else {
                                        findd.pop_back();
                                        findd += ((editorChar != j + 1)
                                                      ? string(character, 1)
                                                      : "") +
                                                 colors[syntaxType];
                                    }
                                } else {
                                    parenthesisCount++;
                                    int c = 0;
                                    for (int z = 0; z < parenthesisCount; z++) {
                                        c++;
                                        if (c == 9)
                                            c = 1;
                                    }
                                    findd.pop_back();
                                    findd += parenthesisColors[to_string(c)] +
                                             ((editorChar != j + 1)
                                                  ? string(character, 1)
                                                  : "") +
                                             colors[syntaxType];
                                }
                            } else {
                                bracketCount++;
                                int c = 0;
                                for (int z = 0; z < bracketCount; z++) {
                                    c++;
                                    if (c == 9)
                                        c = 1;
                                }
                                findd.pop_back();
                                findd += parenthesisColors[to_string(c)] +
                                         ((editorChar != j + 1)
                                              ? string(character, 1)
                                              : "") +
                                         colors[syntaxType];
                            }
                        } else if (find(endParenthesis.begin(),
                                        endParenthesis.end(),
                                        string(character, 1)) !=
                                   endParenthesis.end()) {
                            if (character != '}') {
                                if (character == '>') {
                                    if (!(editorbufferbuffer[j - 1] == ' ' &&
                                          editorbufferbuffer[j + 1] == ' ') &&
                                        parenthesisCount > 0) {
                                        int c = 0;
                                        for (int z = 0; z < parenthesisCount;
                                             z++) {
                                            c++;
                                            if (c == 9)
                                                c = 1;
                                            findd.pop_back();
                                            findd +=
                                                parenthesisColors[to_string(
                                                    c)] +
                                                ((editorChar != j + 1)
                                                     ? string(character, 1)
                                                     : "") +
                                                colors[syntaxType];
                                            parenthesisCount--;
                                        }
                                    }
                                } else if (parenthesisCount > 0) {
                                    int c = 0;
                                    for (int z = 0; z < parenthesisCount; z++) {
                                        c++;
                                        if (c == 9)
                                            c = 1;
                                        findd.pop_back();
                                        findd +=
                                            parenthesisColors[to_string(c)] +
                                            ((editorChar != j + 1)
                                                 ? string(character, 1)
                                                 : "") +
                                            colors[syntaxType];
                                        parenthesisCount--;
                                    }
                                }
                            } else if (bracketCount > 0) {
                                int c = 0;
                                for (int z = 0; z < bracketCount; z++) {
                                    c++;
                                    if (c == 9)
                                        c = 1;
                                    findd.pop_back();
                                    findd += parenthesisColors[to_string(c)] +
                                             ((editorChar != j + 1)
                                                  ? string(character, 1)
                                                  : "") +
                                             colors[syntaxType];
                                    bracketCount--;
                                }
                            }
                        }
                        if (character == '"') {
                            if (quote) {
                                findd += "\033[38;5;34m";
                                while (true) {
                                    j++;
                                    if (j > editorbufferbuffer.length() - 1)
                                        break;
                                    if (editorChar == j + 1)
                                        findd += "\033[46;1m";
                                    character = editorbufferbuffer[j];
                                    findd += character;
                                    if (editorChar == j + 1)
                                        findd += "\033[0m" + themeAnsi +
                                                 "\033[38;5;34m";
                                    if (character == '"') {
                                        findd += colors[syntaxType];
                                        break;
                                    }
                                }
                            } else {
                                findd.pop_back();
                                findd += "\033[38;5;34m\"";
                                while (true) {
                                    j++;
                                    if (j > editorbufferbuffer.length() - 1)
                                        break;
                                    character = editorbufferbuffer[j];
                                    findd += character;
                                    if (character == '"') {
                                        findd += colors[syntaxType];
                                        break;
                                    }
                                }
                            }
                        }
                        if (character == '\'') {
                            if (quote) {
                                findd += "\033[38;5;34m";
                                while (true) {
                                    j++;
                                    if (j > editorbufferbuffer.length() - 1)
                                        break;
                                    if (editorChar == j + 1)
                                        findd += "\033[46;1m";
                                    character = editorbufferbuffer[j];
                                    findd += character;
                                    if (editorChar == j + 1)
                                        findd += "\033[0m" + themeAnsi +
                                                 "\033[38;5;34m";
                                    if (character == '\'') {
                                        findd += colors[syntaxType];
                                        break;
                                    }
                                }
                            } else {
                                findd.pop_back();
                                findd += "\033[38;5;34m'";
                                while (true) {
                                    j++;
                                    if (j > editorbufferbuffer.length() - 1)
                                        break;
                                    character = editorbufferbuffer[j];
                                    findd += character;
                                    if (character == '\'') {
                                        findd += colors[syntaxType];
                                        break;
                                    }
                                }
                            }
                        }
                        if (character == '/') {
                            j--;
                            if (j > editorbufferbuffer.length() - 1) {
                                j++;
                                if (editorbufferbuffer[j + 1] != '/') {
                                    if (editorChar == j + 1)
                                        findd += "\033[46;1m/\033[0m";
                                    else
                                        findd += character;
                                }
                            } else if (editorbufferbuffer[j] == '/') {
                                j++;
                                if (comment) {
                                    findd += themeAnsi + "\033[38;5;8m";
                                    while (true) {
                                        j++;
                                        if (j > editorbufferbuffer.length() - 1)
                                            break;
                                        if (editorChar == j + 1)
                                            findd += "\033[46;1m";
                                        character = editorbufferbuffer[j];
                                        findd += character;
                                        if (editorChar == j + 1)
                                            findd += "\033[0m" + themeAnsi +
                                                     "\033[38;5;34m";
                                    }
                                } else {
                                    findd =
                                        regex_replace(findd, regex("/+$"), "");
                                    if (editorChar == j)
                                        findd += "\033[46;1m\033[38;5;8m/"
                                                 "\033[0m\033[38;5;8m" +
                                                 themeAnsi + "/";
                                    else if (editorChar == j + 1)
                                        findd += "\033[38;5;8m/\033[46;1m/"
                                                 "\033[0m";
                                    else
                                        findd += "\033[38;5;8m//";
                                    while (true) {
                                        j++;
                                        if (j > editorbufferbuffer.length() - 1)
                                            break;
                                        findd += editorbufferbuffer[j];
                                    }
                                    break;
                                }
                            } else {
                                j++;
                                if (editorbufferbuffer[j + 1] != '/') {
                                    if (editorChar == j + 1)
                                        findd += "\033[46;1m/\033[0m";
                                    else
                                        findd += character;
                                }
                            }
                        }
                    }
                    editorbufferline =
                        "\033[38;5;8m" +
                        string(getlinecount -
                                   to_string(editorCurrentLine).length(),
                               ' ') +
                        "\033[93m" + to_string(editorCurrentLine) +
                        "\033[1;38;5;8m    |\033[0m" + themeAnsi + findd +
                        "\033[0m" + themeAnsi + filler + "\033[0m" + themeAnsi +
                        "\n";
                }
            }
            editorbuffer += editorbufferline;
        }
#ifdef _WIN32
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
#else
        cout << "\033[H\033[3J";
#endif
        cout << editorbuffer;
        taskbar = "\033[48;2;56;113;228m\033[35;1m | Save | New Sprite | Open "
                  "File |" +
                  string(terminalwidth - (35 + taskbarMessage.length()), ' ') +
                  taskbarMessage + "\033[0m";
        cout << taskbar;
    };

    auto shiftcursor = [&]() {
        while (inEditor) {
            cursorBlink = false;
            sleep(500);
            cursorBlink = true;
            sleep(500);
        }
    };

    auto editorprintloop = [&]() {
        while (inEditor) {
            getdimensions(terminalheight, terminalwidth);
            editorprint(terminalheight);
        }
    };

    auto onkeypress = [&](string key) {
        string newLine;
        if (find(begin(validKeys), end(validKeys), key) != end(validKeys) ||
            key.length() == 1) {
            string transfer;
            int leadings;
            if (key == "F1") {
                inEditor = false;
                state = "tree";
            } else if (key == "F2") {
                inEditor = false;
                state = "new";
            } else if (key == "UpArrow") {
                taskbarMessage = "Up Arrow";
                editorCurrentLine--;
                if (editorCurrentLine == realLine && realLine - 1)
                    realLine -= 1 + ((editorCurrentLine != 1) ? 1 : 0);
                if (editorCurrentLine < 1) {
                    editorCurrentLine = 1;
                    realLine = 1;
                }
                cursorBlink = true;
                if (editorChar > editorLines[editorCurrentLine - 1].length()) {
                    editorChar = editorLines[editorCurrentLine - 1].length();
                }
            } else if (key == "DownArrow") {
                taskbarMessage = "Down Arrow";
                editorCurrentLine++;
                if (editorCurrentLine == realLine + terminalheight - 3 &&
                    (editorCurrentLine != editorLines.size()))
                    realLine++;
                else if (editorCurrentLine == realLine + (terminalheight - 2))
                    realLine +=
                        1 + ((editorCurrentLine != editorLinesCount) ? 1 : 0);
                cursorBlink = true;
                if (editorCurrentLine > editorLines.size())
                    editorCurrentLine = editorLines.size();
                if (editorChar > editorLines[editorCurrentLine - 1].length())
                    editorChar = editorLines[editorCurrentLine - 1].length();
            } else if (key == "LeftArrow") {
                taskbarMessage = "Left Arrow";
                if (editorChar > 1) {
                    editorChar--;
                } else if (editorCurrentLine > 1) {
                    editorChar = editorLines[editorCurrentLine - 2].length();
                    editorCurrentLine--;
                    if (editorCurrentLine == realLine && realLine > 1)
                        realLine--;
                    else if (editorCurrentLine == realLine - 1)
                        realLine -= 1 + ((editorCurrentLine != 1) ? 1 : 0);
                    if (editorChar >
                        editorLines[editorCurrentLine - 1].length())
                        editorChar =
                            editorLines[editorCurrentLine - 1].length();
                }
                if (quoteComplete)
                    quoteComplete = false;
                if (singleQuoteComplete)
                    singleQuoteComplete = false;
                cursorBlink = true;
            } else if (key == "RightArrow") {
                taskbarMessage = "Right Arrow";
                if (editorChar < editorLines[editorCurrentLine - 1].length())
                    editorChar++;
                else if (editorCurrentLine < editorLinesCount) {
                    editorChar = 1;
                    editorCurrentLine++;
                    if (editorCurrentLine == realLine + terminalheight - 3 &&
                        (editorCurrentLine != editorLinesCount))
                        realLine++;
                    else if (editorCurrentLine ==
                             realLine + (terminalheight - 2))
                        realLine +=
                            1 +
                            ((editorCurrentLine != editorLinesCount) ? 1 : 0);
                    if (editorChar >
                        editorLines[editorCurrentLine - 1].length())
                        editorChar =
                            editorLines[editorCurrentLine - 1].length();
                }
                if (quoteComplete)
                    quoteComplete = false;
                if (singleQuoteComplete)
                    singleQuoteComplete = false;
                cursorBlink = true;
            } else if (key == "Backspace") {
                taskbarMessage = "Backspace";
                if (editorChar == 1 && editorCurrentLine != 1) {
                    editorChar = editorLines[editorCurrentLine - 2].length();
                    transfer = regex_replace(editorLines[editorCurrentLine - 2],
                                             regex(" +$"), "") +
                               editorLines[editorCurrentLine - 1];
                    editorLines[editorCurrentLine - 2] = transfer;
                    editorLinesWithSyntax[editorCurrentLine - 2] =
                        AddSyntax(transfer, false);
                    editorLines.erase(editorLines.begin() +
                                      (editorCurrentLine - 1));
                    editorLinesWithSyntax.erase(editorLinesWithSyntax.begin() +
                                                (editorCurrentLine - 1));
                    editorCurrentLine--;
                    if (realLine > 1 && editorCurrentLine == realLine &&
                        realLine > 1)
                        realLine--;
                } else {
                    string backspaceSplit =
                        editorLines[editorCurrentLine - 1].erase(editorChar - 2,
                                                                 1);
                    editorLines[editorCurrentLine - 1] = backspaceSplit;
                    editorLinesWithSyntax[editorCurrentLine - 1] =
                        AddSyntax(backspaceSplit, false);
                    if (editorChar > 1)
                        editorChar--;
                    editorChar =
                        (editorChar >
                         editorLines[editorCurrentLine - 1].length())
                            ? editorLines[editorCurrentLine - 1].length()
                            : editorChar;
                    cursorBlink = true;
                }
            } else if (key == "Delete") {
                taskbarMessage = "Delete";
                if (editorChar + 1 !=
                    editorLines[editorCurrentLine - 1].length()) {
                    string backspaceSplit =
                        editorLines[editorCurrentLine - 1].erase(editorChar - 1,
                                                                 1);
                    editorLines[editorCurrentLine - 1] = backspaceSplit;
                    editorLinesWithSyntax[editorCurrentLine - 1] =
                        AddSyntax(backspaceSplit, false);
                }
                cursorBlink = true;
            } else if (key == "Enter") {
                taskbarMessage = "Enter";
                leadings = editorLines[editorCurrentLine - 1].length() -
                           regex_replace(editorLines[editorCurrentLine - 1],
                                         regex("^ +"), "")
                               .length();
                if (editorChar == editorLines[editorCurrentLine - 1].length() &&
                    editorLines[editorCurrentLine - 1].length() ==
                        regex_replace(editorLines[editorCurrentLine - 1],
                                      regex("^ +"), "")
                            .length()) {
                    leadings--;
                    if (regex_replace(editorLines[editorCurrentLine - 1],
                                      regex("^ +| +$"), "$1") == "") {
                        editorLines.insert(
                            editorLines.begin() + editorCurrentLine,
                            string(leadings + ((leadings == -1) ? 2 : 0), ' '));
                    } else {
                        editorLines.insert(
                            editorLines.begin() + editorCurrentLine,
                            string(leadings + ((leadings == -1) ? 2 : 1), ' '));
                    }
                    if (regex_replace(editorLines[editorCurrentLine - 1],
                                      regex("^ +| +$"), "$1") == "") {
                        editorLinesWithSyntax.insert(
                            editorLinesWithSyntax.begin() + editorCurrentLine,
                            string(leadings + ((leadings == -1) ? 2 : 0), ' '));
                    } else {
                        editorLinesWithSyntax.insert(
                            editorLinesWithSyntax.begin() + editorCurrentLine,
                            string(leadings + ((leadings == -1) ? 2 : 1), ' '));
                    }
                    editorChar = editorLines[editorCurrentLine].length();
                } else {
                    if (editorChar < 2) {
                        string lineInsert =
                            editorLines[editorCurrentLine - 1].erase(
                                0, (editorChar - 1));
                        editorLines[editorCurrentLine - 1] =
                            editorLines[editorCurrentLine - 1].erase(
                                editorLines[editorCurrentLine - 1].find(
                                    lineInsert),
                                lineInsert.length());
                        editorLinesWithSyntax[editorCurrentLine - 1] =
                            AddSyntax(editorLines[editorCurrentLine - 1],
                                      false);
                        editorLines.insert(
                            editorLines.begin() + editorCurrentLine,
                            string(leadings + ((leadings == -1) ? 2 : 0), ' ') +
                                lineInsert);
                        editorLinesWithSyntax.insert(
                            editorLinesWithSyntax.begin() + editorCurrentLine,
                            AddSyntax(
                                string(leadings + ((leadings == -1) ? 2 : 0),
                                       ' ') +
                                    lineInsert,
                                false));
                        editorChar = leadings + 1;
                    } else {
                        if (editorLines[editorCurrentLine - 1]
                                       [editorChar - 2] == '{') {
                            leadings =
                                editorLines[editorCurrentLine - 1].length() -
                                regex_replace(
                                    editorLines[editorCurrentLine - 1],
                                    regex(" +$"), "")
                                    .length();
                            string lineInsert =
                                string(leadings, ' ') +
                                editorLines[editorCurrentLine - 1].erase(
                                    0, (editorChar - 1));
                            editorLines[editorCurrentLine - 1] =
                                editorLines[editorCurrentLine - 1].erase(
                                    editorLines[editorCurrentLine - 1].find(
                                        lineInsert),
                                    lineInsert.length());
                            editorLinesWithSyntax[editorCurrentLine - 1] =
                                AddSyntax(editorLines[editorCurrentLine - 1],
                                          false);
                            editorLines.insert(
                                editorLines.begin() + editorCurrentLine,
                                string(leadings + tabSize + 1, ' '));
                            editorLinesWithSyntax.insert(
                                editorLinesWithSyntax.begin() +
                                    editorCurrentLine,
                                string(leadings + tabSize + 1, ' '));
                            editorLines.insert(editorLines.begin() +
                                                   (editorCurrentLine + 1),
                                               lineInsert);
                            editorLinesWithSyntax.insert(
                                editorLines.begin() + (editorCurrentLine + 1),
                                AddSyntax(lineInsert, false));
                            editorChar = leadings + tabSize + 1;
                            if (editorCurrentLine ==
                                realLine + terminalheight - 3) {
                                realLine++;
                            }
                        } else {
                            string lineInsert =
                                editorLines[editorCurrentLine - 1].erase(
                                    0, (editorChar - 1));
                            editorLines[editorCurrentLine - 1] =
                                editorLines[editorCurrentLine - 1].erase(
                                    editorLines[editorCurrentLine - 1].find(
                                        lineInsert),
                                    lineInsert.length());
                            editorLinesWithSyntax[editorCurrentLine - 1] =
                                AddSyntax(editorLines[editorCurrentLine - 1],
                                          false);
                            editorLines.insert(
                                editorLines.begin() + editorCurrentLine,
                                string(leadings + ((leadings == -1) ? 2 : 0),
                                       ' ') +
                                    lineInsert);
                            editorLinesWithSyntax.insert(
                                editorLinesWithSyntax.begin() +
                                    editorCurrentLine,
                                AddSyntax(string(leadings +
                                                     ((leadings == -1) ? 2 : 0),
                                                 ' ') +
                                              lineInsert,
                                          false));
                            editorChar = leadings + 1;
                        }
                    }
                }
                editorCurrentLine++;
                if (editorCurrentLine == realLine + terminalheight - 2 ||
                    editorCurrentLine == realLine + terminalheight - 3)
                    realLine++;
                cursorBlink = true;
            } else if (key == "Tab") {
                taskbarMessage = "Tab";
                newLine = editorLines[editorCurrentLine - 1].insert(
                    editorChar - 1, string(tabSize, ' '));
                editorLines[editorCurrentLine - 1] = newLine;
                editorLinesWithSyntax[editorCurrentLine - 1] =
                    AddSyntax(newLine, false);
                editorChar += tabSize;
                cursorBlink = true;
            } else if (key == "PageUp") {
                taskbarMessage = "Page Up";
                realLine -= terminalheight - 3;
                if (realLine > editorLinesCount)
                    realLine = editorLinesCount;
                editorCurrentLine = realLine;
                cursorBlink = true;
            } else if (key == "PageDown") {
                taskbarMessage = "Page Down";
                realLine += terminalheight - 3;
                if (realLine < 1)
                    realLine = 1;
                editorCurrentLine = realLine;
                cursorBlink = true;
            } else {
                taskbarMessage = key;
                if (key == "\"") {
                    if (!quoteComplete) {
                        newLine = editorLines[editorCurrentLine - 1].insert(
                            editorChar - 1, key);
                        editorLines[editorCurrentLine - 1] = newLine;
                        editorLinesWithSyntax[editorCurrentLine - 1] =
                            AddSyntax(newLine, false);
                        editorChar++;
                        cursorBlink = true;
                        if ((editorLines[editorCurrentLine - 1]
                                        [editorChar - 3] == ' ' &&
                             (editorLines[editorCurrentLine - 1]
                                         [editorChar - 1] == ' ')) ||
                            find(startParenthesis.begin(),
                                 startParenthesis.end(),
                                 string(editorLines[editorCurrentLine - 1]
                                                   [editorChar - 3],
                                        1)) != startParenthesis.end()) {
                            quoteComplete = true;
                            newLine = editorLines[editorCurrentLine - 1].insert(
                                editorChar - 1, key);
                            editorLines[editorCurrentLine - 1] = newLine;
                            editorLinesWithSyntax[editorCurrentLine - 1] =
                                AddSyntax(newLine, false);
                        }
                    } else {
                        editorChar++;
                        cursorBlink = true;
                        quoteComplete = false;
                    }
                } else if (key == "'") {
                    if (!singleQuoteComplete) {
                        newLine = editorLines[editorCurrentLine - 1].insert(
                            editorChar - 1, key);
                        editorLines[editorCurrentLine - 1] = newLine;
                        editorLinesWithSyntax[editorCurrentLine - 1] =
                            AddSyntax(newLine, false);
                        editorChar++;
                        cursorBlink = true;
                        if (editorLines[editorCurrentLine - 1]
                                       [editorChar - 3] == ' ') {
                            singleQuoteComplete = true;
                            newLine = editorLines[editorCurrentLine - 1].insert(
                                editorChar - 1, key);
                            editorLines[editorCurrentLine - 1] = newLine;
                            editorLinesWithSyntax[editorCurrentLine - 1] =
                                AddSyntax(newLine, false);
                        }
                    } else {
                        editorChar++;
                        cursorBlink = true;
                        singleQuoteComplete = false;
                    }
                } else if (find(startParenthesis.begin(),
                                startParenthesis.end(),
                                key) != startParenthesis.end()) {
                    parenthesisComplete++;
                    string previousKey = key;
                    newLine = editorLines[editorCurrentLine - 1].insert(
                        editorChar - 1, key);
                    editorLines[editorCurrentLine - 1] = newLine;
                    editorLinesWithSyntax[editorCurrentLine - 1] =
                        AddSyntax(newLine, false);
                    editorChar++;
                    cursorBlink = true;
                    if (key == "(")
                        key = ")";
                    else if (key == "[")
                        key = "]";
                    else if (key == "{")
                        key = "}";
                    else if (key == "<")
                        key = ">";
                    newLine = editorLines[editorCurrentLine - 1].insert(
                        editorChar - 1, key);
                    key = previousKey;
                    editorLines[editorCurrentLine - 1] = newLine;
                    editorLinesWithSyntax[editorCurrentLine - 1] =
                        AddSyntax(newLine, false);
                } else if (find(endParenthesis.begin(), endParenthesis.end(),
                                key) != endParenthesis.end()) {
                    if (parenthesisComplete > 0) {
                        parenthesisComplete--;
                        editorChar++;
                        cursorBlink = true;
                    } else {
                        newLine = editorLines[editorCurrentLine - 1].insert(
                            editorChar - 1, key);
                        editorLines[editorCurrentLine - 1] = newLine;
                        editorLinesWithSyntax[editorCurrentLine - 1] =
                            AddSyntax(newLine, false);
                        editorChar++;
                        cursorBlink = true;
                    }
                } else {
                    newLine = editorLines[editorCurrentLine - 1].insert(
                        editorChar - 1, key);
                    editorLines[editorCurrentLine - 1] = newLine;
                    editorLinesWithSyntax[editorCurrentLine - 1] =
                        AddSyntax(newLine, false);
                    editorChar++;
                    cursorBlink = true;
                }
            }
            if (editorChar < 1)
                editorChar = 1;
            if (editorCurrentLine < 1) {
                editorCurrentLine = 1;
                realLine = 1;
            }
            if (editorCurrentLine > editorLinesCount)
                editorCurrentLine = editorLinesCount;
            if (editorChar > editorLines[editorCurrentLine - 1].length())
                editorChar = editorLines[editorCurrentLine - 1].length();
        }
    };

    auto editorloop = [&]() {
        while (inEditor)
            onkeypress(keypress());
    };

    thread sc(shiftcursor);
    thread epl(editorprintloop);

    while (true) {
        if (state == "edit") {
            inEditor = true;
            editorloop();
        }
    }
    setcursorvisibility(true);
}