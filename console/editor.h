#pragma once

#include <iomanip>
#include <Windows.h>
#include <fstream>
#include <string>
#include <locale>
#include <codecvt>


class Editor {

	std::wstring file_name_;

	std::vector<TextLine*> lines_;
	std::wstring console_msg_;

	std::pair<int,int> cursor_position_;
	int print_line_position_;
private:
	bool OpenFile();
	bool SaveFile();

	void Parser(const std::wstring file_data);
	void PrintLine(const int offset);
	void ChangeText(const std::wstring source, const std::wstring dest);

	int GetTextLine(const std::wstring source);
	void setMsg(const std::wstring msg);
	const std::wstring getMsg();

	void setCursor(const int print_line_count);
public:
	Editor(const std::wstring file_name);
	bool Start();
	bool Save();

	bool InputMsg(const std::wstring msg);
	void Print();
};