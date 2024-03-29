#pragma once
#include "Base.h"

namespace Kaidel {


	template<typename _Key,typename _Value>
	class BiMap{
		using key_type = std::remove_all_extents_t<_Key>;
		using value_type = std::remove_all_extents_t<_Value>;
		using key_reference_type = key_type&;
		using value_reference_type = value_type&;
		using const_key_reference_type = const key_type&;
		using const_value_reference_type = const value_type&;
		using current_direction = BiMap<key_type, value_type>;
		using reverse_direction = BiMap<value_type, key_type>;
		static_assert(!std::is_same_v<key_type, value_type>, "_Key and _Value must be different");
	public:
		BiMap() = default;
		~BiMap() = default;

		BiMap(const current_direction& other) : m_KV(other.m_KV), m_VK(other.m_VK) {}
		BiMap(const reverse_direction& other) : m_KV(other.m_VK), m_VK(other.m_VK) {}


		BiMap(current_direction&& other) noexcept : m_KV(std::move(other.m_KV)), m_VK(std::move(other.m_VK)) {}
		BiMap(reverse_direction&& other) noexcept : m_KV(std::move(other.m_VK)), m_VK(std::move(other.m_KV)) {}

		BiMap& operator=(const current_direction& other) {
			if (this != &other) {
				m_KV = other.m_KV;
				m_VK = other.m_VK;
			}
			return *this;
		}

		BiMap& operator=(const reverse_direction& other) {
			if (this != &other) {
				m_KV = other.m_VK;
				m_VK = other.m_KV;
			}
			return *this;
		}


		BiMap& operator=(current_direction&& other) noexcept {
			if (this != &other) {
				m_KV = std::move(other.m_KV);
				m_VK = std::move(other.m_VK);
			}
			return *this;
		}

		BiMap& operator=(reverse_direction&& other) noexcept {
			if (this != &other) {
				m_KV = std::move(other.m_VK);
				m_VK = std::move(other.m_KV);
			}
			return *this;
		}

		std::unordered_map<key_type, value_type>& KV() { return m_KV; }
		const std::unordered_map<key_type, value_type>& KV() const{ return m_KV; }

		std::unordered_map<value_type, key_type>& VK() { return m_VK; }
		const std::unordered_map<value_type, key_type>& VK() const { return m_VK; }

		void Add(const_key_reference_type k, const_value_reference_type v) {
			m_KV[k] = v;
			m_VK[v] = k;
		}

		void Remove(const_key_reference_type k) {
			auto& v = m_KV.at(k);
			m_KV.erase(k);
			m_VK.erase(v);
		}

		void Remove(const_value_reference_type v) {
			auto& k = m_VK.at(v);
			m_VK.erase(v);
			m_KV.erase(k);
		}


		bool Exists(const_key_reference_type k)const {
			return m_KV.find(k) != m_KV.end();
		}

		bool Exists(const_value_reference_type v)const {
			return m_VK.find(v) != m_VK.end();
		}

		const_key_reference_type operator[](const_value_reference_type v) {

			if (!Exists(v)) {
				Add({}, v);
			}

			return m_VK.at(v);
		}

		const_value_reference_type operator[](const_key_reference_type k) {

			if (!Exists(k)) {
				Add(k, {});
			}

			return m_KV.at(k);
		}

		decltype(auto) begin() const{
			return m_KV.begin();
		}

		decltype(auto) end() const{
			return m_KV.end();
		}

		decltype(auto) cbegin() const{
			return m_KV.cbegin();
		}
		decltype(auto) cend() const {
			return m_KV.cend();
		}


	private:
		std::unordered_map<key_type, value_type> m_KV;
		std::unordered_map<value_type, key_type> m_VK;
	};
}
