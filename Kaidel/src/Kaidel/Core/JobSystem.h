#pragma once
#include <functional>
namespace Kaidel {

	struct JobDispatchArgs {
		uint32_t jobIndex;
		uint32_t groupIndex;
	};

	template <typename T, size_t capacity>
	class ThreadSafeRingBuffer
	{
	public:
		// Push an item to the end if there is free space
		//  Returns true if succesful
		//  Returns false if there is not enough space
		inline bool push_back(const T& item)
		{
			bool result = false;
			std::scoped_lock<std::mutex> lock(this->lock);
			size_t next = (head + 1) % capacity;
			if (next != tail)
			{
				data[head] = item;
				head = next;
				result = true;
			}
			return result;

		}

		// Get an item if there are any
		//  Returns true if succesful
		//  Returns false if there are no items
		inline bool pop_front(T& item)
		{
			bool result = false;
			std::scoped_lock<std::mutex> lock(this->lock);
			if (tail != head)
			{
				item = data[tail];
				tail = (tail + 1) % capacity;
				result = true;
			}
			return result;

		}
		ThreadSafeRingBuffer()
			:lock{}
		{
		}
	private:
		T data[capacity];
		size_t head = 0;
		size_t tail = 0;
		std::mutex lock; // this just works better than a spinlock here (on windows)
	};

	class JobSystem {
	public:

		static void InitMainJobSystem() { s_JobSystem = new JobSystem(16); }

		JobSystem(uint32_t workerThreadCount);
		~JobSystem();
		void Execute(const std::function<void()>& job);
		void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job);
		bool IsBusy();
		void Wait();
		static JobSystem& GetMainJobSystem() { return *s_JobSystem; }
	private:
		void poll();
		struct JobSystemData {
			uint32_t numThreads = 0;    // number of worker threads, it will be initialized in the Initialize() function
			ThreadSafeRingBuffer<std::function<void()>, 256> jobPool;    // a thread safe queue to put pending jobs onto the end (with a capacity of 256 jobs). A worker thread can grab a job from the beginning
			std::condition_variable wakeCondition;    // used in conjunction with the wakeMutex below. Worker threads just sleep when there is no job, and the main thread can wake them up
			std::mutex wakeMutex;    // used in conjunction with the wakeCondition above
			uint64_t currentLabel = 0;    // tracks the state of execution of the main thread
			std::atomic<uint64_t> finishedLabel;    // track the state of execution across background worker threads
		};
		JobSystemData m_Data;
		static JobSystem* s_JobSystem;
	};


}
