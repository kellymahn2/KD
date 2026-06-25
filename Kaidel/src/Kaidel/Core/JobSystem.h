#pragma once
#include <functional>
namespace Kaidel {
	struct JobArgs {
		void* SharedMemory;
		uint32_t GlobalID;
		uint32_t LocalID;
		uint32_t GroupID;
		bool FirstJobInGroup;
		bool LastJobInGroup;
	};

	using Task = std::function<void(const JobArgs&)>;

	enum class DispatchPriority {
		High,
		Low,
		Default = High,
		Count
	};

	struct JobSystemContext {
		std::atomic<uint32_t> ActiveCount = 0;
		DispatchPriority Priority = DispatchPriority::Default;

		JobSystemContext(DispatchPriority priority = DispatchPriority::Default)
			: Priority(priority)
		{
		}

		bool Finished();

		void Wait();
	};

	class JobSystem {
	public:
		static void Init(uint32_t maxThreadCount = ~0U);
		static void Shutdown();

		static void Execute(JobSystemContext& ctx, Task task);

		static void Dispatch(JobSystemContext& ctx, uint32_t jobCount, uint32_t groupSize, Task task, uint64_t sharedMemorySize = 0);

		static void Execute(Task task);

		static void Dispatch(uint32_t jobCount, uint32_t groupSize, Task task, uint64_t sharedMemorySize = 0);

		static JobSystemContext& GetDefault();
	};


}
