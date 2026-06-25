#include "KDpch.h"
#include "JobSystem.h"
namespace Kaidel {

	struct Job
	{
		std::function<void(JobArgs)> Task;
		JobSystemContext* Context;
		uint32_t GroupID;
		uint32_t GroupJobOffset;
		uint32_t GroupJobEnd;
		uint32_t SharedMemorySize;
		
		inline uint32_t Execute()
		{
			JobArgs args;
			args.GroupID = GroupID;
			if (SharedMemorySize > 0)
			{
				args.SharedMemory = alloca(SharedMemorySize);
				memset(args.SharedMemory, 0, SharedMemorySize);
			}
			else
			{
				args.SharedMemory = nullptr;
			}

			for (uint32_t j = GroupJobOffset; j < GroupJobEnd; ++j)
			{
				args.GlobalID = j;
				args.LocalID = j - GroupJobOffset;
				args.FirstJobInGroup = (j == GroupJobOffset);
				args.LastJobInGroup = (j == GroupJobEnd - 1);
				Task(args);
			}

			return Context->ActiveCount.fetch_sub(1);
		}
	};

	struct JobQueue
	{
		std::deque<Job> Queue;
		std::mutex Mutex;

		inline void push_back(const Job& item)
		{
			std::scoped_lock lock(Mutex);
			Queue.push_back(item);
		}
		inline bool pop_front(Job& item)
		{
			std::scoped_lock lock(Mutex);
			if (Queue.empty())
			{
				return false;
			}
			item = std::move(Queue.front());
			Queue.pop_front();
			return true;
		}
	};

	struct PriorityResources
	{
		uint32_t ThreadCount = 0;
		std::unique_ptr<JobQueue[]> JobQueuePerThread;
		std::atomic<uint32_t> NextQueue{ 0 };
		std::condition_variable SleepingCondition; // for workers that are sleeping
		std::mutex SleepingMutex; // for workers that are sleeping
		std::condition_variable WaitingCondition; // for unblocking a Wait()
		std::mutex WaitingMutex; // for unblocking a Wait()

		// Start working on a job queue
		//	After the job queue is finished, it can switch to an other queue and steal jobs from there
		inline void Work(uint32_t startingQueue)
		{
			Job job;
			for (uint32_t i = 0; i < ThreadCount; ++i)
			{
				JobQueue& job_queue = JobQueuePerThread[startingQueue % ThreadCount];
				while (job_queue.pop_front(job))
				{
					uint32_t progress_before = job.Execute();
					if (progress_before == 1)
					{
						// This is likely the last job because the counter was 1 before it was decremented in execute()
						//	So wake up the waiting threads here
						std::unique_lock<std::mutex> lock(WaitingMutex);
						WaitingCondition.notify_all();
					}
				}
				startingQueue++; // go to next queue
			}
		}
	};


	struct JobSystemData {
		uint32_t NumCores = 0;
		PriorityResources Resources[(uint32_t)DispatchPriority::Count];
		JobSystemContext GlobalContext;
	} static* s_Data;

	
	bool JobSystemContext::Finished()
	{
		return ActiveCount.load(std::memory_order_acquire) == 0;
	}

	void JobSystemContext::Wait()
	{
		if (!Finished())
		{
			PriorityResources& resources = s_Data->Resources[(uint32_t)Priority];

			resources.SleepingCondition.notify_all();

			resources.Work(resources.NextQueue.fetch_add(1) & resources.ThreadCount);

			while (!Finished())
			{
				std::unique_lock<std::mutex> lock(resources.WaitingMutex);

				if (!Finished())
				{
					resources.WaitingCondition.wait(lock, [this]() { return Finished(); });
				}
			}
		}
	}

	void JobSystem::Init(uint32_t maxThreadCount /*= ~0U*/)
	{
		maxThreadCount = std::max(1U, maxThreadCount);

		s_Data = new JobSystemData;

		s_Data->NumCores = std::thread::hardware_concurrency();

		for (uint32_t i = 0; i < (uint32_t)DispatchPriority::Count; ++i)
		{
			DispatchPriority priority = (DispatchPriority)i;
			
			PriorityResources& resources = s_Data->Resources[i];

			switch (priority)
			{
			case Kaidel::DispatchPriority::High: resources.ThreadCount = s_Data->NumCores - 1; break;
			case Kaidel::DispatchPriority::Low: resources.ThreadCount = s_Data->NumCores - 2; break;
			}

			resources.ThreadCount = std::clamp(resources.ThreadCount, 1U, maxThreadCount);
			
			resources.JobQueuePerThread.reset(new JobQueue[resources.ThreadCount]);

			for (uint32_t thread = 0; thread < resources.ThreadCount; ++thread)
			{
				std::thread worker([&resources, thread]() 
				{
					while (true)
					{
						resources.Work(thread);

						std::unique_lock<std::mutex> lock(resources.SleepingMutex);

						resources.SleepingCondition.wait(lock);
					}
				}
				);

				auto handle = worker.native_handle();

				int core = thread + 1;
#ifdef _WIN32
				// Do Windows-specific thread setup:

				// Put each thread on to dedicated core:
				DWORD_PTR affinityMask = 1ull << core;
				DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);
				assert(affinity_result > 0);

				if (priority == DispatchPriority::High)
				{
					BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
					assert(priority_result != 0);

					std::wstring wthreadname = L"Job_" + std::to_wstring(thread);
					HRESULT hr = SetThreadDescription(handle, wthreadname.c_str());
					assert(SUCCEEDED(hr));
				}
				else if (priority == DispatchPriority::Low)
				{
					BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_LOWEST);
					assert(priority_result != 0);

					std::wstring wthreadname = L"JobL" + std::to_wstring(thread);
					HRESULT hr = SetThreadDescription(handle, wthreadname.c_str());
					assert(SUCCEEDED(hr));
				}
#endif

				worker.detach();
			}

		}
	}

	void JobSystem::Shutdown()
	{
		delete s_Data;
	}

	void JobSystem::Execute(JobSystemContext& ctx, Task task)
	{
		PriorityResources& resources = s_Data->Resources[(uint32_t)ctx.Priority];

		// Context state is updated:
		ctx.ActiveCount.fetch_add(1);

		Job job;
		job.Context = &ctx;
		job.Task = task;
		job.GroupID = 0;
		job.GroupJobOffset = 0;
		job.GroupJobEnd = 1;
		job.SharedMemorySize = 0;

		resources.JobQueuePerThread[resources.NextQueue.fetch_add(1) % resources.ThreadCount].push_back(job);
		resources.SleepingCondition.notify_one();
	}

	void JobSystem::Execute(Task task)
	{
		Execute(s_Data->GlobalContext, task);
	}

	void JobSystem::Dispatch(JobSystemContext& ctx, uint32_t jobCount, uint32_t groupSize, Task task, uint64_t sharedMemorySize /*= 0*/)
	{
		if (jobCount == 0 || groupSize == 0)
		{
			return;
		}
		PriorityResources& res = s_Data->Resources[(uint32_t)ctx.Priority];

		const uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;

		// Context state is updated:
		ctx.ActiveCount.fetch_add(groupCount);

		Job job;
		job.Context = &ctx;
		job.Task = task;
		job.SharedMemorySize = (uint32_t)sharedMemorySize;

		for (uint32_t groupID = 0; groupID < groupCount; ++groupID)
		{
			// For each group, generate one real job:
			job.GroupID = groupID;
			job.GroupJobOffset = groupID * groupSize;
			job.GroupJobEnd = std::min(job.GroupJobOffset + groupSize, jobCount);

			{
				res.JobQueuePerThread[res.NextQueue.fetch_add(1) % res.ThreadCount].push_back(job);
			}
		}

		if (res.ThreadCount > 1)
		{
			res.SleepingCondition.notify_all();
		}
	}

	void JobSystem::Dispatch(uint32_t jobCount, uint32_t groupSize, Task task, uint64_t sharedMemorySize /*= 0*/)
	{
		Dispatch(s_Data->GlobalContext, jobCount, groupSize, task, sharedMemorySize);
	}

	Kaidel::JobSystemContext& JobSystem::GetDefault()
	{
		return s_Data->GlobalContext;
	}

}

