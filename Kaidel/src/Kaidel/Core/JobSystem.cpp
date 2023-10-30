#include "KDpch.h"
#include "JobSystem.h"
namespace Kaidel {
	
	

	JobSystem::JobSystem(uint32_t workerThreadCount)
	{
		// Initialize the worker execution state to 0:
		m_Data.finishedLabel.store(0);

		// Retrieve the number of hardware threads in this system:
		auto numCores = std::thread::hardware_concurrency();

		// Calculate the actual number of worker threads we want:
		m_Data.numThreads = std::max(workerThreadCount, numCores);

		// Create all our worker threads while immediately starting them:
		for (uint32_t threadID = 0; threadID < m_Data.numThreads; ++threadID)
		{
			std::thread worker ([this] {

				std::function<void()> job; // the current job for the thread, it's empty at start.

				// This is the infinite loop that a worker thread will do 
				while (true)
				{
					if (m_Data.jobPool.pop_front(job)) // try to grab a job from the jobPool queue
					{
						// It found a job, execute it:
						job(); // execute job
						m_Data.finishedLabel.fetch_add(1); // update worker label state
					}
					else
					{
						// no job, put thread to sleep
						std::unique_lock<std::mutex> lock(m_Data.wakeMutex);
						m_Data.wakeCondition.wait(lock);
					}
				}

				});

			// *****Here we could do platform specific thread setup...
			worker.detach();

		}

	}

	JobSystem::~JobSystem()
	{

	}

	void JobSystem::poll()
	{
		m_Data.wakeCondition.notify_one(); // wake one worker thread
		std::this_thread::yield(); // allow this thread to be rescheduled
	}
	void JobSystem::Execute(const std::function<void()>& job)
	{
		// The main thread label state is updated:
		m_Data.currentLabel += 1;

		// Try to push a new job until it is pushed successfully:
		while (!m_Data.jobPool.push_back(job)) { poll(); }

		m_Data.wakeCondition.notify_one(); // wake one thread
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
		m_Data.currentLabel += groupCount;

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
			while (!m_Data.jobPool.push_back(jobGroup)) { poll(); }

			m_Data.wakeCondition.notify_one(); // wake one thread
		}
	}

	bool JobSystem::IsBusy()
	{
		return m_Data.finishedLabel.load() < m_Data.currentLabel;
	}

	void JobSystem::Wait()
	{
		while (IsBusy()) { poll(); }
	}

}

