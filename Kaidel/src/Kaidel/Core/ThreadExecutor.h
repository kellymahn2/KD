#include <functional>
#include <vector>
#include <thread>
namespace Kaidel{
	

	
	using namespace std::chrono_literals;
	enum ThreadFunctionType {
		Blocking = 1<<0,
		LastNecessary = 1<<1
	};
	class ThreadExecutor {
	public:
		ThreadExecutor(int numThreads);
		void Execute();
		void AddFunction(std::function<void()>&& func,int flags=0);
	private:
		struct ThreadFunction {
			ThreadFunction(std::function<void()>&& func, int flags);
			ThreadFunction(ThreadFunction&&);
			ThreadFunction() = default;
			std::function<void()> Func;
			std::atomic<bool> Resolved;
			void operator()() {
				Func();
				Resolved = true;
			}
			int Flags = 0;
		};
		struct ThreadDefinition {
			std::thread Thread;
			std::mutex Blocked;
			std::atomic<bool> Finished  = true;
			ThreadDefinition() = default;
			ThreadDefinition(ThreadDefinition&&);
			ThreadDefinition(std::function<void()>&&);
			void Block() {
				Blocked.lock();
			}
			void UnBlock() {
				Blocked.unlock();
			}
			void Join() {
				while (Finished.load() == false)
					std::this_thread::sleep_for(.002ms);
			}
		};
		std::vector<ThreadFunction> m_Functions;
		int m_ThreadCount;
		std::atomic<size_t> m_CurrentFunctionIndex;
		std::vector<ThreadDefinition> m_Threads;

		void CreateThreads();

		void JoinThreads();

		void ExecuteFunctions();

		void ThreadFunction(int threadIndex);
		void CallFunctions(int threadIndex);
	};
}
