#pragma once
#include <KDpch.h>
namespace Kaidel {

	struct Buffer {
		void* Data;
		uint64_t Size;
		Buffer() = default;
		Buffer(const Buffer&) = default;
		template<typename T>
		Buffer(T* data, uint64_t size)
			:Data(static_cast<void*>(const_cast<T*>(data))),Size(size)
		{
		}

		template<typename T>
		std::_Remove_cvref_t<T>* As()const { return static_cast<std::_Remove_cvref_t<T>*>(Data); }
	};

	template<typename T>
	class ScopedBuffer {
	public:
		ScopedBuffer() = default;
		ScopedBuffer(const ScopedBuffer&) = default;
		ScopedBuffer(ScopedBuffer& rhs) {
			m_Buffer = rhs.m_Buffer;
		}
		T* Get()const {
			return m_Buffer.As<T>();
		}
		uint64_t Size()const {
			return m_Buffer.Size;
		}
	private:
		Buffer m_Buffer;
	};

}
