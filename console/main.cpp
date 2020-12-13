// console.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "stdafx.h"

WordPool* WordPool::instance_ = nullptr;

int main()
{
    Editor *editor = new Editor(L"test.txt");
	editor->Start();

	while (true)
	{
		editor->Print();
		std::wstring input_string;
		std::wcin >> input_string;
		if (!editor->InputMsg(input_string))
			break;
	}
}
