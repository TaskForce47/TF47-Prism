#pragma once

#include <queue>

#include "intercept.hpp"

namespace tf47::prism::helper
{
	template<typename T>
	class thread_safe_queue
	{
	private:
		std::queue<T> queue_;
		std::recursive_mutex lock_;
	public:
		thread_safe_queue() {}

		T pop()
		{
			std::lock_guard lock(lock_);
			auto item = queue_.front();
			queue_.pop();
			return item;
		}

		void push(T data)
		{
			std::lock_guard lock(lock_);
			queue_.push(data);
		}

		bool empty()
		{
			std::lock_guard lock(lock_);
			return queue_.empty();
		}
	};
}