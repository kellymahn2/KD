#pragma once
#include <functional>
namespace Kaidel {

	struct JobDispatchArgs {
		uint32_t jobIndex;
		uint32_t groupIndex;
	};
	class JobSystem {
	public:
		JobSystem(uint32_t workerThreadCount);
		~JobSystem();
		void Execute(const std::function<void()>& job);
		void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job);
		bool IsBusy();
		void Wait();



	private:




	};


}
