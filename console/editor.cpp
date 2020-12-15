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
		if (print_line_position_ == 0)
			setMsg(L"This is the front page!");
		else
			print_line_position_ = ((print_line_position_ - 20) > 0) ? (print_line_position_ - 20 ): 0;
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
		if(!IsDigit(str_line) || !IsDigit(str_position))
		{
				setMsg(L"Input value is not valid");
				return true;
		}

		int line = std::stoi(str_line)-1 + print_line_position_;
		int position = std::stoi(str_position)-1;

		if (line < 0 || line >= lines_.size())
		{
			setMsg(L"Input value is not valid");
			return true;
		}

		auto temp = lines_[line];
		if (temp->PopPosition(position))
			DeleteRearrange(line);
		else
			setMsg(L"Input value is not valid");
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

					if(!IsDigit(str_line) || !IsDigit(str_position))
					{
						setMsg(L"Input value is not valid");
						return true;
					}

					int line = std::stoi(str_line)-1 + print_line_position_;
					int position = std::stoi(str_position)-1;
					if (line < 0 || line >= lines_.size())
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
					temp->SearchPush(position, text);
					InsertRearrange(line);
					return true;
				}
			}
		}
		setMsg(L"Input value is not valid");
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
		i->SaveWords(file);
		WordPool::GetInstance().Free(i);
	}
	file.close();

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
	size_t now_position = 0, end_position = 0;
	bool is_end = false;
	while (true)
	{
		TextLine* temp = WordPool::GetInstance().GetAlloc();
		while (temp->GetByte() <= 75)
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
			temp->Push(file_data.substr(now_position, str_size));
			
			now_position = end_position + 1;

			if (is_end)
				break;
		}
		lines_.push_back(temp);
		if (is_end)
			return;
	}

}

void Editor::PrintLine(const int offset)
{
	lines_[print_line_position_ + offset]->Print();
}

void Editor::ChangeText(const std::wstring source, const std::wstring dest)
{
	for (auto i : lines_)
		i->ChangeWord(source, dest);
}

int Editor::GetTextLine(const std::wstring source)
{
	for (int i = 0; i < lines_.size(); i++)
	{
		if (lines_[i]->SearchText(source))
			return i;
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
	if (lines_[start_line]->GetByte() >= 75)
		return;

	for (int i = 0; (start_line + i) < (lines_.size()-1); i++)
	{
		int now_line = i + start_line;
		if (lines_[now_line]->GetByte() >= 75)
			break;

		while (lines_[now_line]->GetByte() < 75)
		{
			auto retval = lines_[now_line + 1]->Pop(false);
			if (retval == L"")
				break;
			lines_[now_line]->Push(retval);
		}
	}
}

void Editor::InsertRearrange(const int start_line)
{
	if (lines_[start_line]->GetByte() <= 75)
		return;

	for (int i = 0; (start_line + i) < (lines_.size() - 1); i++)
	{
		int now_line = i + start_line;
		if (lines_[now_line]->GetByte() <= 75)
			break;

		while (lines_[now_line]->GetByte() > 75)
		{
			auto retval = lines_[now_line]->Pop();
			if (retval == L"")
				break;
			lines_[now_line + 1]->Push(retval, false);
		}
	}
}

bool Editor::IsDigit(const std::wstring str)
{
	for (auto i : str)
	{
		if (!isdigit(i))
			return false;
	}

	return true;
}