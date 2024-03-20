#pragma once
#include "net_common.h"

namespace net
{
	template<typename T>
	class tsqueue
	{
	public:
		tsqueue() = default;
		tsqueue(const tsqueue&) = delete;
		virtual ~tsqueue() { clear(); }

		const T& front()
		{
			std::lock_guard<std::mutex> guard(muxQueue);
			return deqQueue.front();
		}

		const T& back()
		{
			std::lock_guard<std::mutex> guard(muxQueue);
			return deqQueue.back();
		}

		void push_back(const T& element)
		{
			std::lock_guard<std::mutex> guard(muxQueue);
			deqQueue.push_back(element);

			// waking up the cv sleeping in wait() function
			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.notify_one();
		}

		void push_front(const T& element)
		{
			std::lock_guard<std::mutex> guard(muxQueue);
			deqQueue.emplace_front(element);

			// waking up the cv sleeping in wait() function
			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.notify_one();
		}

		bool empty()
		{
			std::lock_guard<std::mutex> guard(muxQueue);
			return deqQueue.empty();
		}

		size_t size()
		{
			std::lock_guard<std::mutex> guard(muxQueue);
			return deqQueue.size();
		}

		void clear()
		{
			std::lock_guard<std::mutex> guard(muxQueue);
			deqQueue.clear();
		}

		T pop_front()
		{
			std::lock_guard<std::mutex> guard(muxQueue);
			auto item = std::move(deqQueue.front());
			deqQueue.pop_front();
			return item;
		}

		T pop_back()
		{
			std::lock_guard<std::mutex> guard(muxQueue);
			auto item = std::move(deqQueue.back());
			deqQueue.pop_back();
			return item;
		}

		void wait()
		{
			while (empty())
			{
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.wait(ul);
			}
		}

	protected:
		std::mutex muxQueue;
		std::deque<T> deqQueue;

		std::condition_variable cvBlocking;
		std::mutex muxBlocking;
	};
}