#pragma once
#include <functional>
#include <vector>
namespace Kaidel {
	template<typename T>
	struct BoundedVector_Iterator {
		T& operator[](uint64_t index) {
			//Add Assert here.
			return *(m_Start + index);
		}
	private:
		BoundedVector_Iterator(T* start, uint64_t size) {
			m_Start = start;
			m_Size = size;
		}
		T* m_Start;
		uint64_t m_Size;
		template<class U>
		friend class BoundedVector;
	};

	template<typename T>
	class BoundedVector {
	public:
		BoundedVector() = default;
		BoundedVector(uint64_t startSize, uint64_t maxSize, std::function<void(T*, uint64_t)>&& overflowFunc) {
			//Add Assert so that startSize is always less then maxSize.
			m_Internal.reserve(startSize);
			m_MaxSize = maxSize;
			m_OverflowFunction = std::move(overflowFunc);
			m_ActualEnd = m_Internal.begin()._Ptr;
		}
		BoundedVector_Iterator<T> Reserve(uint64_t size) {
			uint64_t newSize = (m_ActualEnd - m_Internal.begin()._Ptr) + size;
			//Make sure we dont go the max size
			if (newSize > m_MaxSize) {
				ProcessOverflow();
				newSize -= m_MaxSize;
			}
			//Make sure we have the space for the inserts.
			uint64_t oldSize = m_ActualEnd - m_Internal.begin()._Ptr;
			if (newSize > m_Internal.capacity()) {
				m_Internal = std::move(CopyData(newSize));
			}
			T* oldEnd = m_Internal.begin()._Ptr + oldSize;
			m_ActualEnd = m_Internal.begin()._Ptr + newSize;
			return BoundedVector_Iterator<T>(oldEnd, size);
		}
		bool CanReserveWithoutOverflow(uint64_t size) {
			uint64_t newSize = (m_ActualEnd - m_Internal.begin()._Ptr) + size;
			//Make sure we dont go the max size
			if (newSize > m_MaxSize) {
				return false;
			}
			return true;
		}
		void Reset() {
			m_ActualEnd = m_Internal.begin()._Ptr;
		}

		uint64_t Size() const {
			return m_ActualEnd - m_Internal.begin()._Ptr;
		}
		T* Get() {
			return m_Internal.data();
		}
	private:
		std::vector<T> CopyData(uint64_t newSize) {
			std::vector<T> res;
			uint64_t oldSize = m_ActualEnd - m_Internal.begin()._Ptr;
			res.resize(newSize);
			T* start = res.data();
			for (uint64_t i = 0; i < oldSize; ++i) {
				*(start + i) = *(Get() + i);
			}
			return res;
		}
		void ProcessOverflow() {
			m_OverflowFunction(Get(), m_ActualEnd - m_Internal.begin()._Ptr);
			m_ActualEnd = m_Internal.begin()._Ptr;
		}
		std::vector<T> m_Internal;
		std::function<void(T*, uint64_t)> m_OverflowFunction;
		uint64_t m_MaxSize = 0;
		T* m_ActualEnd = nullptr;
		template<class U>
		friend struct BoundedVector_Iterator;
	};
}
