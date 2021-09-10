#pragma once

#include <mutex>
#include <optional>
#include <queue>

namespace Acorn::Utils
{
	template <typename T>
	class ThreadSafeQueue
	{

		bool Empty() const
		{
			return m_Queue.empty();
		}

	public:
		ThreadSafeQueue() = default;
		ThreadSafeQueue(const ThreadSafeQueue&) = delete;
		ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

		ThreadSafeQueue(ThreadSafeQueue<T>&& other)
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Queue = std::move(other.m_Queue);
		}

		virtual ~ThreadSafeQueue(){};

		unsigned long Size() const
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			return m_Queue.size();
		}

		std::optional<T> Pop()
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			if (m_Queue.empty())
			{
				return {};
			}
			auto item = m_Queue.front();
			m_Queue.pop();
			return item;
		}

		void Push(const T& item)
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Queue.push(item);
		}

	private:
		mutable std::mutex m_Mutex;
		std::queue<T> m_Queue;
	};
}