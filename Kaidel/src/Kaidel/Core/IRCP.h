#pragma once
#include <atomic>


namespace Kaidel {
	template<bool ThreadSafe>
	struct IRCCounter;

	template<>
	struct IRCCounter<false> {
		uint64_t Count = 0;
		void IncrementRef() {
			++Count;
		}
		template<typename T>
		void DecrementRef() {
			if (--Count == 0) {
				delete dynamic_cast<T*>(this);
			}
		}

		virtual ~IRCCounter() = default;
	};

	template<>
	struct IRCCounter<true> {
		std::atomic<uint64_t> Count = 0;

		void IncrementRef() {
			++Count;
		}
		template<typename T>
		void DecrementRef() {
			if (--Count == 0) {
				delete dynamic_cast<T*>(this);
			}
		}

		virtual ~IRCCounter() = default;
	};


	template<typename T, bool ThreadSafe = false>
	class IRCPointer {
		static_assert(std::is_base_of_v<IRCCounter<ThreadSafe>, T>, "T must be derived from IRCCounter");

	public:
		IRCPointer() : m_Ptr(nullptr) {}

		IRCPointer(T* ptr) : m_Ptr(ptr) {
			if (m_Ptr)
				m_Ptr->IncrementRef();
		}

		IRCPointer(const IRCPointer& other) : m_Ptr(other.m_Ptr) {
			if (m_Ptr)
				m_Ptr->IncrementRef();
		}

		IRCPointer(IRCPointer&& other) noexcept : m_Ptr(other.m_Ptr) {
			other.m_Ptr = nullptr;
		}

		template<typename U>
		IRCPointer(const IRCPointer<U, ThreadSafe>& other){
			m_Ptr = (T*)other.m_Ptr;
			if (m_Ptr)
				m_Ptr->IncrementRef();
		}

		IRCPointer& operator=(const IRCPointer& other) {
			if (this != &other) {
				if (m_Ptr)
					m_Ptr->DecrementRef<T>();

				m_Ptr = other.m_Ptr;

				if (m_Ptr)
					m_Ptr->IncrementRef();
			}
			return *this;
		}

		IRCPointer& operator=(IRCPointer&& other) noexcept {
			if (this != &other) {
				if (m_Ptr)
					m_Ptr->DecrementRef<T>();

				m_Ptr = other.m_Ptr;
				other.m_Ptr = nullptr;
			}
			return *this;
		}

		template<typename U>
		IRCPointer& operator=(const IRCPointer<U, ThreadSafe>& other) {
			if ((void*)this != (void*)&other) {
				if (m_Ptr)
					m_Ptr->DecrementRef<T>();

				m_Ptr = (T*)other.m_Ptr;

				if (m_Ptr)
					m_Ptr->IncrementRef();
			}
			return *this;
		}



		template<typename U>
		IRCPointer& operator=(IRCPointer<U, ThreadSafe>&& other) noexcept {
			if ((void*)this != (void*)&other) {
				if (m_Ptr)
					m_Ptr->DecrementRef<T>();

				m_Ptr = (T*)other.m_Ptr;
				other.m_Ptr = nullptr;
			}
			return *this;
		}

		~IRCPointer() {
			if (m_Ptr)
				m_Ptr->DecrementRef<T>();
		}
		template<typename U>
		bool operator!=(const IRCPointer<U, ThreadSafe>& other) const {
			return (void*)m_Ptr != (void*)other.m_Ptr;
		}

		template<typename U>
		bool operator==(const IRCPointer<U, ThreadSafe>& other) const {
			return (void*)m_Ptr == (void*)other.m_Ptr;
		}
		operator bool() const {
			return m_Ptr != nullptr;
		}
		T* Get()const { return m_Ptr; }
		T& operator*()const { return *m_Ptr; }
		T* operator->()const { return m_Ptr; }

	private:
		T* m_Ptr;

	private:
		template<typename To, typename From, bool ThreadSafe>
		friend IRCPointer<To, ThreadSafe> DynamicPointerCast(const IRCPointer<From, ThreadSafe>& ptr);

		template<typename U, bool Safe, typename... Args>
		friend IRCPointer<U, Safe> CreateIRCPointer(Args&&... args);

		friend class IRCPointer;

	};



	template<typename To, typename From, bool ThreadSafe>
	IRCPointer<To, ThreadSafe> DynamicPointerCast(const IRCPointer<From, ThreadSafe>& ptr) {
		return IRCPointer<To, ThreadSafe>(dynamic_cast<To*>(ptr.m_Ptr));
	}

	template<typename T, bool ThreadSafe, typename... Args>
	IRCPointer<T, ThreadSafe> CreateIRCPointer(Args&&... args) {
		return IRCPointer<T, ThreadSafe>(new T(std::forward<Args>(args)...));
	}


	template<typename T>
	using IRCP = IRCPointer<T, false>;

}
namespace std {
	template<typename T, bool ThreadSafe>
	struct hash<Kaidel::IRCPointer<T, ThreadSafe>> {
		size_t operator()(const Kaidel::IRCPointer<T, ThreadSafe>& ptr) const {
			// Hash the raw pointer value
			return hash<void*>{}(ptr.Get());
		}
	};
}
