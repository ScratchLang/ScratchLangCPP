// ScratchlangCPP.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include <conio.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <Windows.h>
#include <miniz/miniz.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

// ANSI Color codes
extern const char* const RED = "\033[0;31m";
extern const char* const NC = "\033[0m";
extern const char* const P = "\033[0;35m";

void addfiletozip(const char* filename, mz_zip_archive& archive, const char* buffer)
{
	FILE* file = fopen(filename, "rb");
	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* file_data = new char[file_size];
	fread(file_data, 1, file_size, file);
	mz_zip_writer_add_mem(&archive, filename, file_data, file_size, MZ_DEFAULT_COMPRESSION);
	delete[] file_data;
	fclose(file);
}

void createziparchive(const char* folder_name, const char* zip_name)
{
	mz_zip_archive archive;
	memset(&archive, 0, sizeof(archive));
	mz_bool status = mz_zip_writer_init_file(&archive, zip_name, 0);
	if (!status)
	{
		printf("Failed to create archive\n");
		return;
	}
	const size_t buffer_size = 1024 * 1024;
	char* buffer = new char[buffer_size];
	DIR* dir = opendir(folder_name);
	if (!dir)
	{
		printf("Failed to open folder\n");
		return;
	}
	dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}
		char path[PATH_MAX];
		snprintf(path, PATH_MAX, "%s/%s", folder_name, entry->d_name);
		addfiletozip(entry->d_name, archive, buffer);
	}
	closedir(dir);
	mz_zip_writer_finalize_archive(&archive);
	mz_zip_writer_end(&archive);
	delete[] buffer;
}

void extract_zip_file(const char* zip_name, const char* dest_folder)
{
	mz_zip_archive archive;
	memset(&archive, 0, sizeof(archive));
	filesystem::create_directory(dest_folder);
	int num_files = mz_zip_reader_get_num_files(&archive);
	for (int i = 0; i < num_files; i++)
	{
		mz_zip_archive_file_stat file_stat;
		mz_zip_reader_file_stat(&archive, i, &file_stat);
		char dest_path[PATH_MAX];
		snprintf(dest_path, PATH_MAX, "%s/%s", dest_folder, file_stat.m_filename);
	}
	mz_zip_reader_end(&archive);
}

void compiler()
{
}

void decompiler()
{
}

void error(string text)
{
	cout << RED << "Error: " << text << NC << endl;
}

void error(const char* text)
{
	cout << RED << "Error: " << text << NC << endl;
}

char getinput(string message = "")
{
	if (message != "")
		cout << message << endl;
	return (char)_getch();
}

void writetofile(string file, char const* towrite)
{
	bool exists = filesystem::exists(file);
	ofstream openfile(file, ios::app);
	if (exists)
	{
		openfile << "\n" << towrite;
		return;
	}
	openfile << towrite;
}

string getexecwd()
{
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