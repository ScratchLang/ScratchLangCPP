// ScratchlangCPP.cpp : Defines the entry point for the application.
//
#define CURL_STATICLIB
#include <scratchlangfunctions.h>
#include <zip_file.hpp>
#include <algorithm>
#include <atlstr.h>
#include <conio.h>
#include <cstring>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

void inputloop(string ia1 = "");

static size_t writedata(void const* ptr, size_t size, size_t nmemb, FILE* stream)
{
	return fwrite(ptr, size, nmemb, stream);
}

void startloop(const char* a1 = "", int argus = 0, char* arguv[] = {})
{
	filesystem::path p = getexecwd();
	string realcwd = p.parent_path().string();
	filesystem::current_path((filesystem::path)(realcwd + "\\data"));
	fstream f("version", ios::in);
	string ver;
	getline(f, ver);
	f.close();
	filesystem::current_path("mainscripts");
	if (!filesystem::exists("var\\asked") && !filesystem::exists("var\\vc"))
	{
		if (tolower(getinput("Would you like ScratchLang to check its version every time you start it? [Y/N]")) == 'y')
			writetofile("var\\vc", "Don't remove this file please.");
		writetofile("var\\asked", "Don't remove this file please.");
	}
	if (filesystem::exists("var\\vc") && a1 != "nope")
	{
		cout << "Checking version..." << endl;
		if (filesystem::exists("version"))
			remove("version");
		CURL* curl;
		static const char* filename = "version";
		FILE* file;
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/ScratchLang/ScratchLang/main/version");
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);
		file = fopen(filename, "wb");
		if (file)
		{
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
			curl_easy_perform(curl);
			fclose(file);
		}
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		string utd = "1";
		string gver;
		fstream f("version", ios::in);
		getline(f, gver);
		f.close();
		if (gver == "")
		{
			error("Checking version failed.");
			Sleep(3000);
		}
		else
		{
			filesystem::remove("version");
			if (ver != gver)
				utd = "0";
			if (utd == "0" && tolower(getinput("Your version of ScratchLang (" + ver + ") is outdated. The current version is " + gver + ". Would you like to update? [Y/N]")) == 'y')
			{
				system("git pull origin main");
				exit(0);
			}
		}
	}
	bool args = false;
	bool args2 = false;
	if (argus > 1)
	{
		if (strcmp(arguv[1], "--help") == 0)
		{
			cout << "Usage: scratchlang.exe [OPTIONS]\n" << endl
				<< "  -1                Create a project." << endl
				<< "  -2                Remove a project." << endl
				<< "  -3                Compile a project." << endl
				<< "  -4                Decompile a project." << endl
				<< "  -5                Export a project." << endl
				<< "  -6                Import a project." << endl
				<< "  --debug [FILE]    Debug a ScratchScript file. Currently not available." << endl
				<< "  --help            Display this help message." << endl
				<< "  --edit            Edit a ScratchLang project.\n" << endl
				<< "Examples:" << endl
				<< "  .\\scratchlang.exe -4 C:\\Users\\Me\\project.sb3" << endl;
			exit(0);
		}
		args = true;
		if (argus > 2)
			args2 = true;
	}
	system("cls");
	cout << P << "\n      /$$$$$$                                 /$$               /$$       /$$                                    \n"
		<< "     /$$__  $$                               | $$              | $$      | $$                                    \n"
		<< "    | $$  \\__/  /$$$$$$$  /$$$$$$  /$$$$$$  /$$$$$$    /$$$$$$$| $$$$$$$ | $$        /$$$$$$  /$$$$$$$   /$$$$$$ \n"
		<< "    |  $$$$$$  /$$_____/ /$$__  $$|____  $$|_  $$_/   /$$_____/| $$__  $$| $$       |____  $$| $$__  $$ /$$__  $$\n"
		<< "     \\____  $$| $$      | $$  \\__/ /$$$$$$$  | $$    | $$      | $$  \\ $$| $$        /$$$$$$$| $$  \\ $$| $$  \\ $$\n"
		<< "     /$$  \\ $$| $$      | $$      /$$__  $$  | $$ /$$| $$      | $$  | $$| $$       /$$__  $$| $$  | $$| $$  | $$\n"
		<< "    |  $$$$$$/|  $$$$$$$| $$     |  $$$$$$$  |  $$$$/|  $$$$$$$| $$  | $$| $$$$$$$$|  $$$$$$$| $$  | $$|  $$$$$$$\n"
		<< "     \\______/  \\_______/|__/      \\_______/   \\___/   \\_______/|__/  |__/|________/ \\_______/|__/  |__/ \\____  $$\n"
		<< "                                                                                                        /$$  \\ $$\n"
		<< "                                                                                                       |  $$$$$$/\n"
		<< "                                                                                                        \\______/ \n" << NC << endl; // print the logo
	if (!args || a1 == "nope")
	{
		cout << "Welcome to ScratchLang" << ver << ". (Name suggested by @MagicCrayon9342 on Scratch.)" << endl;
		inputloop();
	}
	else
	{
		if (strcmp(arguv[1], "--edit") == 0)
		{
			cout << "Sorry, the editor has not been implemented yet." << endl;
			exit(0);
		}
		else
			inputloop(arguv[1]);
	}
	filesystem::current_path("..");
	if (filesystem::exists("projects"))
	{
		filesystem::current_path("projects");
		filesystem::directory_iterator diriterate(".");
		int filecount = 0;
		for (const auto& _ : filesystem::directory_iterator("."))
			filecount++;
		if (filecount == 0)
		{
			filesystem::current_path("..");
			filesystem::remove("projects");
		}
	}
}

void inputloop(string ia1)
{
	string inp = "";
	int a1 = NULL;
	if (ia1 != "")
	{
		ia1.erase(0, 1);
		try
		{
			a1 = stoi(ia1);
		}
		catch (invalid_argument&)
		{
			error(ia1 + " is not an argument.");
			Sleep(2000);
			startloop("nope");
		}
	}
	if (a1 == NULL)
	{
		cout << "1) Create a project." << endl
			<< "2) Remove a project." << endl
			<< "3) Compile a project." << endl
			<< "4) Decompile a .sb3 file." << endl
			<< "5) Export project." << endl
			<< "6) Import project." << endl
			<< "7) Are options 3 and 4 not working? Input 7 to install dependencies." << endl;
		if (!filesystem::exists("var\\devmode"))
		{
			cout << "8) Enable Developer Mode." << endl
				<< "9) Exit." << endl;
		}
		else
		{
			cout << "8) Disable Developer Mode." << endl
				<< "9) Delete all variables." << endl
				<< "0) Prepare for commit and push." << endl
				<< "-) Exit." << endl;
		}
		inp = getinput();
	}
	else
	{
		if (a1 > 0 && a1 < 7)
			inp = to_string(a1);
		else
		{
			error(ia1 + " is not an argument.");
			Sleep(2000);
			startloop("nope");
		}
	}
	if (inp == "1")
	{
		cout << "\nName your project." << endl;
		string namechar;
		getline(cin, namechar);
		string name = namechar;
		filesystem::current_path("..");
		if (namechar == "")
		{
			error("Project name cannot be empty.");
			exit(0);
		}
		else if (filesystem::exists("projects\\" + name))
		{
			char yessor = getinput("Project" + name + " already exists. Replace? [Y/N]");
			if (tolower(yessor) == 'y')
				filesystem::remove_all(namechar);
			else if (tolower(yessor) == 'n')
				exit(0);
			else
			{
				error(yessor + " is not an input.");
				exit(0);
			}
		}
		cout << "You named your project " + name + ". If you want to rename it, use the File Explorer." << endl;
		filesystem::create_directories("projects\\" + name + "\\Stage\\assets");
		filesystem::copy("resources\\cd21514d0531fdffb22204e0ec5ed84a.svg", "projects\\" + name + "\\Stage\\assets", filesystem::copy_options::overwrite_existing);
		writetofile("projects\\" + name + "\\Stage\\project.ss1", "// There should be no empty lines.");
		filesystem::create_directories("projects\\" + name + "\\Sprite1\\assets");
		filesystem::copy("resources\\341ff8639e74404142c11ad52929b021.svg", "projects\\" + name + "\\Sprite1\\assets", filesystem::copy_options::overwrite_existing);
		writetofile("projects\\" + name + "\\Sprite1\\project.ss1", "// There should be no empty lines.");
		cout << "Sorry, the editor is not available as of this moment." << endl;
	}
	else if (inp == "2")
	{
		filesystem::current_path("..");
		if (!filesystem::exists("projects"))
		{
			error("There are no projects to delete.");
			exit(0);
		}
		filesystem::current_path("projects");
		cout << endl;
		for (const auto& file : filesystem::directory_iterator("."))
			cout << filesystem::canonical(file.path()) << endl;
		cout << endl;
		string ddrd;
		getline(cin, ddrd);
		if (filesystem::path pgrd = ddrd; pgrd != "")
		{
			if (filesystem::exists(pgrd))
				filesystem::remove_all(pgrd);
			else
				error("Directory " + pgrd.string() + "does not exist.");
		}
		exit(0);
	}
	else if (inp == "3")
	{
		compiler();
		exit(0);
	}
	else if (inp == "4")
	{
		decompiler();
		exit(0);
	}
	else if (inp == "5")
	{
		filesystem::current_path("..");
		if (!filesystem::exists("projects"))
		{
			error("There are no projects to export.");
			exit(0);
		}
		filesystem::current_path("projects");
		cout << endl;
		for (const auto& file : filesystem::directory_iterator("."))
			cout << filesystem::canonical(file.path()).filename().string() << endl;
		cout << "\nChoose a project to export, or input nothing to cancel." << endl;
		string ddrd;
		getline(cin, ddrd);
		if (filesystem::path pgrd = ddrd; pgrd != "")
		{
			if (filesystem::exists(pgrd))
			{
				filesystem::current_path("..\\exports");
				miniz_cpp::zip_file ssa;
				ssa.write(filesystem::current_path().parent_path().string() + "\\" + pgrd.string());
				ssa.save(pgrd.string() + ".ssa");
			}
			else
				error("Directory " + pgrd.string() + " does not exist.");
		}
		exit(0);
	}
	else if (inp == "6")
	{
		string ssi = filedialog("Choose a ScratchScript Archive", (filesystem::current_path().parent_path().string() + "\\exports").c_str(), "ScratchScript Archive (*.ssa);;All Files (*.*)");
		filesystem::current_path(((filesystem::path)ssi).parent_path());
		filesystem::rename(ssi, "a.zip");
		miniz_cpp::zip_file ssa;
		ssa.extractall("a.zip");
		filesystem::rename("a.zip", ssi);
	}
}

int main(int argc, char* argv[]) // guaranteed 1 argument, which is command used to execute executable
{
	QApplication app(argc, argv);
	startloop("", argc, argv);
	return QApplication::exec();
}