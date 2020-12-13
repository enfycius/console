#pragma once

#include <queue>

struct TextLine;

struct Node
{
	TextLine *data = new TextLine();
	int use_count = 0;
};

class WordPool
{
private:
	std::queue<Node*> empty_pool_;
	std::queue<Node*> used_pool_;

	int used_count_;
	int empty_count_;
	static WordPool* instance_;

public:
	WordPool()
	{
		used_count_ = 0;
		empty_count_ = 0;
		instance_ = nullptr;
	}

	static WordPool& GetInstance() {
		if (instance_ == nullptr)
		{
			instance_ = new WordPool();
		}
		return *instance_;
	}

	TextLine* GetAlloc()
	{
		Node* data = nullptr;
		if (!empty_pool_.empty())
		{
			data = empty_pool_.front();
			empty_pool_.pop();
		}
		else
			data = new Node();

		used_pool_.push(data);

		empty_count_--;
		used_count_++;

		return data->data;
	}

	void Free(TextLine* str)
	{
		auto temp = used_pool_.front();
		used_pool_.pop();
		empty_pool_.push(temp);

		empty_count_++;
		used_count_--;		
	}
};
