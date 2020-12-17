#include "stdafx.h"
#include "TextLine.h"

TextLine::TextLine()
{
	bytes_ = 0;
	is_enter_ = false;
}

void TextLine::SetByte(const int size, const bool is_plus)
{
	bytes_ += size;
	if (words_.size() > 1)
	{
		if (is_plus)
			bytes_ += 1;
		else
			bytes_ -= 1;
	}
		
}

bool TextLine::Push(const std::wstring str, bool is_back)
{
	// 이 함수 내부에선 byte 사이즈를 체크하지 않는다.
	if (is_back)
		words_.push_back(str);
	else
		words_.push_front(str);

	bytes_ += str.size();
	if (words_.size() > 1)
		bytes_ += 1;

	return true;
}

bool TextLine::SearchPush(const int count, const std::wstring str)
{
	if (words_.empty())
		return false;

	if (count < 0 || count >= words_.size())
		return false;

	auto iter = words_.begin();
	auto end_iter = words_.end();
	for (int i = 0; iter != end_iter; iter++, i++)
	{
		if (i == count)
		{
			iter++;
			break;
		}
	}

	if (iter == end_iter)
		words_.push_back(str);
	else
		words_.insert(iter, str);
	SetByte(str.size());

	return true;
}

const std::wstring TextLine::Pop(bool is_back)
{
	if (words_.empty())
		return L"";

	std::wstring retval = L"";
	if (is_back)
	{
		retval = words_.back();
		words_.pop_back();
	}
	else
	{
		retval = words_.front();
		words_.pop_front();
	}
	SetByte(retval.size() * -1, false);

	return retval;
}

bool TextLine::PopPosition(const int position)
{
	if (position < 0 || position >= words_.size())
		return false;

	auto iter = words_.begin();
	auto end_iter = words_.end();
	for (int i = 0; iter != end_iter; iter++, i++)
	{
		if (i == position)
			break;
	}

	if(iter == words_.end())
		return false;
;
	SetByte(iter->size() * -1, false);
	words_.erase(iter);
	return true;
}

int TextLine::GetByte()
{
	return bytes_;
}

void TextLine::SaveWords(std::wofstream& file)
{
	if ( bytes_== 0)
		file << std::endl;
	else
	{
		for (auto j : words_)
			file << j << L' ';
	}
}

void TextLine::Print()
{
	for (auto i : words_)
		std::wcout << i << ' ';
}

bool TextLine::ChangeWord(const std::wstring source, const std::wstring dest)
{
	for (auto& j :words_)
	{
		if (j == source)
		{
			j = dest;
			return true;
		}
	}
	return false;
}

bool TextLine::SearchText(const std::wstring str)
{
	for (auto i : words_)
	{
		if (i == str)
			return true;
	}
	return false;
}

bool TextLine::GetEnter()
{
	return is_enter_;
}

void TextLine::SetEnter()
{
	is_enter_ = true;
}