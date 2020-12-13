#include "stdafx.h"
#include "editor.h"

Editor::Editor(const std::wstring file_name)
{
	file_name_ = file_name;
	print_line_position_ = 0;
	cursor_position_ = {5,8};
	setlocale(LC_ALL, "ko-KR");
}

bool Editor::Start()
{
	return OpenFile();
}

bool Editor::Save()
{
	if (SaveFile())
		return true;
	return false;
}

bool Editor::InputMsg(std::wstring msg)
{
	if (msg == L"n")
	{
		// Print next page
		if (lines_.size() < (print_line_position_ + 20))
			setMsg(L"This is the last page!");
		else
			print_line_position_ += 20;
		return true;
	}
	else if (msg == L"p")
	{
		if (print_line_position_ - 20 < 0)
			setMsg(L"This is the front page!");
		else
			print_line_position_ -= 20;
		return true;
	}
	else if (msg == L"t")
	{
		return Save();
	}
	else if (msg.front() == L'c')
	{
		std::wstring source = L"", dest = L"";
		size_t split_position = msg.find(',', 0);
		source = msg.substr(2, split_position - 2);
		dest = msg.substr(split_position + 1);
		dest.pop_back();

		// 괄호 안에 공백 있으면 입력 오류 체크
		if (source.find(' ') == std::wstring::npos && dest.find(' ') == std::wstring::npos) 
			ChangeText(source, dest);
		else
			setMsg(L"Please check your input text!");

		return true;
	}
	else if (msg.front() == L's')
	{
		std::wstring str = msg.substr(2);
		str.pop_back();

		if (str.empty() || str.find(' ') != std::wstring::npos)
			setMsg(L"Please check your input text!");
		else
		{
			const int retval = GetTextLine(str);
			if (retval == -1)
			{
				std::wstring error_msg = str + L" is not exist";
				setMsg(error_msg);
				return true;
			}
			else
				print_line_position_ = retval;
		}
		return true;
	}
}

void Editor::Print()
{
	system("cls");
	std::cout.setf(std::ios::right);
	int count = 0;
	for (count; count < 20 && (print_line_position_ + count) < lines_.size(); count++)
	{
		std::cout << std::setw(2) << (count + 1) << "| ";
		PrintLine(count);
		std::cout << std::endl;
	}
	std::wcout <<L"------------------------------------------------------------------------------------------"<<std::endl;
	std::wcout <<L"n : 다음페이지, p: 이전페이지, i : 삽입, d:삭제, c : 변경, s : 찾기, t : 저장후종료" << std::endl;
	std::wcout << L"------------------------------------------------------------------------------------------" << std::endl;
	std::wcout << L"( 콘솔메세지 ) " << getMsg() << std::endl;
	std::wcout << L"------------------------------------------------------------------------------------------" << std::endl;
	std::wcout << L" 입력 : " << std::endl;
	std::wcout << L"------------------------------------------------------------------------------------------" << std::endl;

	setCursor(count);
}

/*
	fstream을 사용하면 \n (개행문자) 입력을 받지 않는다.

*/
bool Editor::OpenFile()
{
	std::wifstream file(file_name_.c_str(),std::ios::in);
	if (!file.is_open())
	{
		std::wstring msg = file_name_ + L"is not exists!";
		setMsg(msg);
		return false;
	}
	std::wstring text;

	while (std::getline(file,text))
		Parser(text);

	file.close();
	return true;
}

bool Editor::SaveFile() 
{
	std::wofstream file(file_name_.c_str(), std::ios::out);
	if (!file.is_open())
	{
		std::wstring msg = file_name_ + L"is not exists!";
		setMsg(msg);
		return false;
	}
	for (auto i : lines_)
	{
		if (i->bytes == 0)
			file << std::endl;
		else 
		{
			for (auto j : i->words)
				file << j << L' ';
		}
	}
	file.close();

	for (auto i : lines_)
	{
		WordPool::GetInstance().Free(i);
	}
	return true;
}

/*
	1. 단순 줄 바꿈인 경우
	2. 문장의 마지막이 개행문자 인 경우
	3. 문장의 마지막이 개행문자가 아니면서, 파일의 끝인경우
*/

void Editor::Parser(const std::wstring file_data)
{
	if (file_data == L"\r")
	{
		lines_.push_back(new TextLine());
		return;
	}
	int line_bytes = 0;
	size_t now_position = 0, end_position = 0;
	bool is_end = false;
	while (true)
	{
		TextLine* temp = WordPool::GetInstance().GetAlloc();
		while (line_bytes <= 75)
		{
			end_position = file_data.find(' ', now_position);
			if (end_position == std::string::npos)
			{
				is_end = true;
				size_t last_position = file_data.find('\r', now_position);
				if (last_position != std::string::npos)
					end_position = last_position;
				else
					end_position = file_data.size();
			}
			size_t str_size = end_position - now_position;
			temp->words.push_back(file_data.substr(now_position, str_size));
			now_position = end_position + 1;
			line_bytes += str_size;
			if (is_end)
				break;
		}
		temp->bytes = line_bytes;
		line_bytes = 0;
		lines_.push_back(temp);
		if (is_end)
			return;
	}

}

void Editor::PrintLine(const int offset)
{
	for (auto i : lines_[print_line_position_ + offset]->words)
		std::wcout << i << ' ';
}

void Editor::ChangeText(const std::wstring source, const std::wstring dest)
{
	for (auto i : lines_)
	{
		for (auto j : i->words)
		{
			if (j == source)
				j = dest;
		}
	}
}

int Editor::GetTextLine(const std::wstring source)
{
	for (int i = 0; i < lines_.size(); i++)
	{
		for (auto j : lines_[i]->words)
		{
			if (j == source)
				return i;
		}
	}
	return -1;
}

void Editor::setMsg(const std::wstring msg)
{
	console_msg_ = msg;
}

const std::wstring Editor::getMsg()
{
	std::wstring ret = console_msg_;
	console_msg_ = L"";
	return ret;
}

void Editor::setCursor(const int print_line_count)
{
	COORD cursor;
	cursor.Y = print_line_count + 5;
	cursor.X = cursor_position_.second;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}