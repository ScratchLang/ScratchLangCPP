// ScratchlangCPP.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include <conio.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <Windows.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>

using namespace std;

// ANSI Color codes
extern const char* const RED = "\033[0;31m";
extern const char* const NC = "\033[0m";
extern const char* const P = "\033[0;35m";

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

void writetofile(string file, char const* toWrite)
{
	bool exists = filesystem::exists(file);
	ofstream openfile(file, ios::app);
	if (exists)
	{
		openfile << "\n" << toWrite;
		return;
	}
	openfile << toWrite;
}

string filedialog(const char* title, const char* start, const char* filter)
{
	if (QString fileName = QFileDialog::getOpenFileName(nullptr, title, start, filter); !fileName.isEmpty())
	{
		return fileName.toLocal8Bit().constData();
	}
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