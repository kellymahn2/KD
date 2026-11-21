#pragma once
#include <atomic>
#include <shared_mutex>

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


	template<typename T>
	class IRCPointer {
		static_assert(std::is_base_of_v<IRCCounter<false>, T> || std::is_base_of_v<IRCCounter<true>, T>, 
			"T must be derived from IRCCounter<true | false>");

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
		IRCPointer(const IRCPointer<U>& other){
			static_assert(std::is_base_of_v<T, U>, "You can only up cast");
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
		IRCPointer& operator=(const IRCPointer<U>& other) {
			static_assert(std::is_base_of_v<T, U>, "You can only up cast");
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
		IRCPointer& operator=(IRCPointer<U>&& other) noexcept {
			static_assert(std::is_base_of_v<T, U>, "You can only up cast");
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
		bool operator!=(const IRCPointer<U>& other) const {
			return (void*)m_Ptr != (void*)other.m_Ptr;
		}

		template<typename U>
		bool operator==(const IRCPointer<U>& other) const {
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
		template<typename To, typename From>
		friend IRCPointer<To> DynamicPointerCast(const IRCPointer<From>& ptr);

		template<typename U, typename... Args>
		friend IRCPointer<U> CreateIRCPointer(Args&&... args);

		friend class IRCPointer;

	};



	template<typename To, typename From>
	IRCPointer<To> DynamicPointerCast(const IRCPointer<From>& ptr) {
		return IRCPointer<To>(static_cast<To*>(ptr.m_Ptr));
	}

	template<typename T, typename... Args>
	IRCPointer<T> CreateIRCPointer(Args&&... args) {
		auto ptr = IRCPointer<T>(new T(std::forward<Args>(args)...));
		ptr->Count = 1;
		return ptr;
	}


	template<typename T>
	using IRCP = IRCPointer<T>;

}
namespace std {
	template<typename T>
	struct hash<Kaidel::IRCPointer<T>> {
		size_t operator()(const Kaidel::IRCPointer<T>& ptr) const {
			// Hash the raw pointer value
			return hash<void*>{}(ptr.Get());
		}
	};
}
