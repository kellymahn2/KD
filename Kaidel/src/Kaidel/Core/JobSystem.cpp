#include "KDpch.h"
#include "JobSystem.h"
namespace Kaidel {
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
			std::unique_lock<std::mutex> lock(this->lock);
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
			std::unique_lock<std::mutex> lock(this->lock);
			if (tail != head)
			{
				item = data[tail];
				tail = (tail + 1) % capacity;
				result = true;
			}
			return result;
		}
		ThreadSafeRingBuffer() = default;
	private:
		T data[capacity];
		size_t head = 0;
		size_t tail = 0;
		std::mutex lock; // this just works better than a spinlock here (on windows)
	};
	struct JobSystemData {
		uint32_t numThreads = 0;    // number of worker threads, it will be initialized in the Initialize() function
		ThreadSafeRingBuffer<std::function<void()>, 256> jobPool;    // a thread safe queue to put pending jobs onto the end (with a capacity of 256 jobs). A worker thread can grab a job from the beginning
		std::condition_variable wakeCondition;    // used in conjunction with the wakeMutex below. Worker threads just sleep when there is no job, and the main thread can wake them up
		std::mutex wakeMutex;    // used in conjunction with the wakeCondition above
		uint64_t currentLabel = 0;    // tracks the state of execution of the main thread
		std::atomic<uint64_t> finishedLabel;    // track the state of execution across background worker threads
	};
	static JobSystemData s_Data;

	JobSystem::JobSystem(uint32_t workerThreadCount)
	{
		// Initialize the worker execution state to 0:
		s_Data.finishedLabel.store(0);

		// Retrieve the number of hardware threads in this system:
		auto numCores = std::thread::hardware_concurrency();

		// Calculate the actual number of worker threads we want:
		s_Data.numThreads = std::max(workerThreadCount, numCores);

		// Create all our worker threads while immediately starting them:
		for (uint32_t threadID = 0; threadID < s_Data.numThreads; ++threadID)
		{
			std::thread worker([] {

				std::function<void()> job; // the current job for the thread, it's empty at start.

				// This is the infinite loop that a worker thread will do 
				while (true)
				{
					if (s_Data.jobPool.pop_front(job)) // try to grab a job from the jobPool queue
					{
						// It found a job, execute it:
						job(); // execute job
						s_Data.finishedLabel.fetch_add(1); // update worker label state
					}
					else
					{
						// no job, put thread to sleep
						std::unique_lock<std::mutex> lock(s_Data.wakeMutex);
						s_Data.wakeCondition.wait(lock);
					}
				}

				});

			// *****Here we could do platform specific thread setup...

			worker.detach(); // forget about this thread, let it do it's job in the infinite loop that we created above

		}

	}
	inline void poll()
	{
		s_Data.wakeCondition.notify_one(); // wake one worker thread
		std::this_thread::yield(); // allow this thread to be rescheduled
	}
	void JobSystem::Execute(const std::function<void()>& job)
	{
		// The main thread label state is updated:
		s_Data.currentLabel += 1;

		// Try to push a new job until it is pushed successfully:
		while (!s_Data.jobPool.push_back(job)) { poll(); }

		s_Data.wakeCondition.notify_one(); // wake one thread
	}

	void JobSystem::Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job)
	{

		if (jobCount == 0 || groupSize == 0)
		{
			return;
		}

		// Calculate the amount of job groups to dispatch (overestimate, or "ceil"):
		const uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;

		// The main thread label state is updated:
		s_Data.currentLabel += groupCount;

		for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
		{
			// For each group, generate one real job:
			auto& jobGroup = [jobCount, groupSize, job, groupIndex]() {

				// Calculate the current group's offset into the jobs:
				const uint32_t groupJobOffset = groupIndex * groupSize;
				const uint32_t groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

				JobDispatchArgs args;
				args.groupIndex = groupIndex;

				// Inside the group, loop through all job indices and execute job for each index:
				for (uint32_t i = groupJobOffset; i < groupJobEnd; ++i)
				{
					args.jobIndex = i;
					job(args);
				}
				};

			// Try to push a new job until it is pushed successfully:
			while (!s_Data.jobPool.push_back(jobGroup)) { poll(); }

			s_Data.wakeCondition.notify_one(); // wake one thread
		}
	}

	bool JobSystem::IsBusy()
	{
		return s_Data.finishedLabel.load() < s_Data.currentLabel;
	}

	void JobSystem::Wait()
	{
		while (IsBusy()) { poll(); }
	}

}

