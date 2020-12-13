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
		return !Save();
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
	else if (msg.front() == L'd')
	{
		/*
			[예외처리]
				1. 공백문자
				2. 숫자가 아닌 경우
				3. (,1) 같은 경우
		*/
		std::wstring str_line, str_position;
		if (msg.size() < 6)
		{
			setMsg(L"Input value is not valid");
			return true;
		}

		size_t split_position = msg.find(',');
		if(split_position == std::wstring::npos || msg.size() < 6)
		{
			// d(,)
			std::wstring error_msg = msg + L" is not valid";
			setMsg(error_msg);
			return true;
		}
		str_line = msg.substr(2, split_position - 2);
		str_position = msg.substr(split_position + 1);
		str_position.pop_back();

		// 입력받은 데이터가 숫자 범위인지 체크
		for (auto i : str_line)
		{
			if (!isdigit(i))
			{
				setMsg(L"Input value is not valid");
				return true;
			}
		}
		for (auto i : str_position)
		{
			if (!isdigit(i))
			{
				setMsg(L"Input value is not valid");
				return true;
			}
		}

		int line = std::stoi(str_line)-1 + print_line_position_;
		int position = std::stoi(str_position)-1;

		if (line < 0 || line >= lines_.size() || position < 0 || position >= lines_[line]->words.size())
		{
			setMsg(L"Input value is not valid");
			return true;
		}

		int count = 0;
		auto temp = lines_[line];
		auto iter = temp->words.begin();
		auto endIter = temp->words.end();
		for (; iter != endIter;iter++, count++)
		{
			if (count == position)
			{
				temp->bytes -= iter->size();
				temp->words.erase(iter);
				break;
			}
		}
		DeleteRearrange(line);
	}
	else if (msg.front() == L'i')
	{
		if (msg.size() < 8)
		{
			setMsg(L"Input value is not valid");
			return true;
		}

		std::wstring str_line, str_position, text;
		size_t split_position = msg.find(',');
		size_t start_position = 2;
		if (split_position != std::wstring::npos )
		{
			str_line = msg.substr(start_position, split_position-start_position);
			start_position = split_position + 1;
			split_position = msg.find(',', start_position);
			if (split_position != std::wstring::npos)
			{
				str_position = msg.substr(start_position, (split_position - start_position));
				start_position = split_position + 1;
				split_position = msg.find(')', start_position);
				if (split_position != std::wstring::npos)
				{
					text = msg.substr(start_position, (split_position - start_position));

					// 입력받은 데이터가 숫자 범위인지 체크
					for (auto i : str_line)
					{
						if (!isdigit(i))
						{
							setMsg(L"Input value is not valid");
							return true;
						}
					}
					for (auto i : str_position)
					{
						if (!isdigit(i))
						{
							setMsg(L"Input value is not valid");
							return true;
						}
					}

					int line = std::stoi(str_line)-1 + print_line_position_;
					int position = std::stoi(str_position)-1;
					if (line < 0 || line >= lines_.size() || position < 0 || position >= lines_[line]->words.size())
					{
						setMsg(L"Input value is not valid");
						return true;
					}

					if (text.size() > 75 || text.empty())
					{
						setMsg(L"Input value is not valid");
						return true;
					}
					// 라인의 제일 마지막 자리에 삽입되는 경우
					auto temp = lines_[line];
					if (position == temp->words.size())
					{
						if (temp->bytes <= 75)
						{
							temp->words.push_back(text);
							temp->bytes += text.size();
							return true;
						}
						lines_[line + 1]->words.push_front(text);
						lines_[line + 1]->bytes += text.size();
						InsertRearrange(line + 1);
						return true;
					}

					int count = 0;
					auto iter = temp->words.begin();
					auto endIter = temp->words.end();
					for (; iter != endIter; iter++, count++)
					{
						if (count == position)
						{
							iter++;
							if (iter == endIter)
								temp->words.push_back(text);
							else
								temp->words.insert(iter, text);
							break;
						}
					}
					temp->bytes += text.size();
					InsertRearrange(line);
					return true;
				}
			}

		}
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

void Editor::DeleteRearrange(const int start_line)
{
	if (lines_[start_line]->bytes >= 75)
		return;

	for (int i = 0; (start_line + i) < (lines_.size()-1); i++)
	{
		int now_line = i + start_line;
		if (lines_[now_line]->bytes >= 75)
			break;

		auto word = lines_[now_line+1]->words.front();
		lines_[now_line+1]->words.pop_front();
		lines_[now_line+1]->bytes -= word.size();

		lines_[now_line]->words.push_back(word);
		lines_[now_line]->bytes += word.size();
	}
}

void Editor::InsertRearrange(const int start_line)
{
	if (lines_[start_line]->bytes <= 75)
		return;

	for (int i = 0; (start_line + i) < (lines_.size() - 1); i++)
	{
		int now_line = i + start_line;
		if (lines_[now_line]->bytes <= 75)
			break;

		auto word = lines_[now_line]->words.back();
		lines_[now_line ]->words.pop_back();
		lines_[now_line ]->bytes -= word.size();

		lines_[now_line+1]->words.push_front(word);
		lines_[now_line+1]->bytes += word.size();
	}
}