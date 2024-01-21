#pragma once
#include <type_traits>
#include <vector>
#include <functional>
namespace Kaidel {

	template<typename _T>
	struct RecursiveTree {
		using node_type = std::remove_all_extents_t<_T>;
		using reference_node_type = node_type&;
		using const_node_type = const node_type;
		using const_reference_node_type = const node_type&;

		using iteration_function = std::function<void(RecursiveTree&)>;
		using const_iteration_function = std::function<void(const RecursiveTree&)>;


		node_type Data;
		RecursiveTree() = default;
		RecursiveTree(const_reference_node_type node)
			:Data(node)
		{}

		RecursiveTree& operator[](std::size_t index) { return SubTrees[index]; }
		const RecursiveTree& operator[](std::size_t index)const { return SubTrees[index]; }
		RecursiveTree& AddChild(const_reference_node_type node) { SubTrees.emplace_back(node); return *this; }
		RecursiveTree& AddChild(const RecursiveTree& node) { SubTrees.push_back(node); return *this; }
		RecursiveTree& Last() { return SubTrees.back(); }
		const RecursiveTree& Last() const { return SubTrees.back(); }

		void PreOrder(iteration_function function) {
			function(*this);
			for (auto& child : SubTrees) {
				child.PreOrder(function);
			}
		}

		void PreOrder(const_iteration_function function) const {
			function(*this);
			for (const auto& child : SubTrees) {
				child.PreOrder(function);
			}
		}


		void PostOrder(iteration_function function) {
			for (auto& child : SubTrees) {
				child.PostOrder(function);
			}
			function(*this);
		}

		void PostOrder(const_iteration_function function) const {
			for (const auto& child : SubTrees) {
				child.PostOrder(function);
			}
			function(*this);
		}


		std::vector<RecursiveTree<_T>> SubTrees{ 0 };
	};

}
