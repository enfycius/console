#pragma once

class TextLine {
private:
	std::list<std::wstring> words_;
	int bytes_;
	bool is_enter;

private:
	void SetByte(const int size, const bool is_plus = true);

public:
	TextLine();
	bool Push(const std::wstring str, bool is_back=true);
	bool SearchPush(const int count, const std::wstring str);
	const std::wstring Pop(bool is_back=true);
	bool PopPosition(const int position);
	int GetByte();

	void SaveWords(std::wofstream& file);
	void Print();
	bool SearchText(const std::wstring str);
	bool ChangeWord(const std::wstring source, const std::wstring dest);

	bool GetEnter();
};
