#include "KDpch.h"
#include "ThreadExecutor.h"
namespace Kaidel {

	ThreadExecutor::ThreadExecutor(int numThreads)
		: m_ThreadCount(numThreads)
	{
		m_CurrentFunctionIndex = 0;
		CreateThreads();
	}


	void ThreadExecutor::Execute()
	{
		ExecuteFunctions();
	}


	void ThreadExecutor::AddFunction(std::function<void()>&& func,int flags)
	{
		m_Functions.emplace_back(std::move(func),flags);
	}

	void ThreadExecutor::CreateThreads()
	{
		m_Threads.reserve(m_ThreadCount);
		for (int i = 0; i < m_ThreadCount; ++i)
		{
			m_Threads.emplace_back([this, i]() { ThreadFunction(i); } );
		}
	}

	void ThreadExecutor::JoinThreads()
	{
		for (auto& thread : m_Threads)
		{
			thread.Join();
		}
		m_Functions.clear();
	}


	void ThreadExecutor::ExecuteFunctions()
	{
		m_CurrentFunctionIndex = 0;
		for (auto& thread : m_Threads) {
			thread.Finished = false;
		}
		JoinThreads();
	}
	ThreadExecutor::ThreadFunction::ThreadFunction(std::function<void()> && func, int flags)
		:Func(func),Flags(flags)
	{

	}
	
	ThreadExecutor::ThreadFunction::ThreadFunction(ThreadFunction&& lhs)
	{
		Func = lhs.Func;
		Flags = lhs.Flags;
	}

	void ThreadExecutor::ThreadFunction(int threadIndex)
	{
		auto& thread = m_Threads[threadIndex];
		thread.Finished = true;
		while (true) {
			while (thread.Finished==true) {
				std::this_thread::sleep_for(.001ms);
			}
			CallFunctions(threadIndex);
			thread.Finished = true;
		}
	}


	void ThreadExecutor::CallFunctions(int threadIndex)
	{
		size_t functionIndex = threadIndex;
		auto& thread = m_Threads.at(threadIndex);
		while (functionIndex < m_Functions.size())
		{
			thread.Block();
			if (m_Functions.at(functionIndex).Flags & Blocking) {
				for (int i = 0; i < m_ThreadCount; ++i) {
					if (i == threadIndex)
						continue;
					m_Threads.at(i).Block();
				}
			}
			if (m_Functions.at(functionIndex).Flags & LastNecessary) {
				for (int i = 0; i < functionIndex; ++i) {
					while (!m_Functions.at(i).Resolved.load());
				}
			}
			m_Functions[functionIndex]();
			if (m_Functions.at(functionIndex).Flags & Blocking) {
				for (int i = 0; i < m_ThreadCount; ++i) {
					if (i == threadIndex)
						continue;
					m_Threads.at(i).UnBlock();
				}
			}
			functionIndex += m_ThreadCount;
			thread.UnBlock();
		}
	}


	ThreadExecutor::ThreadDefinition::ThreadDefinition(std::function<void()>&& func)
		:Thread(func)
	{

	}


	ThreadExecutor::ThreadDefinition::ThreadDefinition(ThreadDefinition&& rhs)
	{
		Thread = std::move(rhs.Thread);
	}

}
