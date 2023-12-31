#pragma once
#include "Kaidel/Core/BoundedVector.h"
#include "VertexTypes.h"
namespace Kaidel {

	template<typename TargetType, typename... TypesAndFunctions>
	struct FunctionChooser;


	template<typename TargetType,typename WrongType,typename WrongFunction,typename... TypesAndFunctions>
	struct FunctionChooser<TargetType,WrongType,WrongFunction,TypesAndFunctions...>  : FunctionChooser<TargetType,TypesAndFunctions...>{

	};
	template<typename TargetType,typename TargetFunction,typename... TypesAndFunctions>
	struct FunctionChooser<TargetType, TargetType, TargetFunction, TypesAndFunctions...> {
		using ValueType = TargetType;
		using FunctionType = TargetFunction;
		FunctionType Func;
		template<typename... Args>
		decltype(auto) operator ()(Args&&...args){
			Func(std::forward<Args>(args)...);
		}

	};


	class Renderer3DFlusher {
	public:
		Renderer3DFlusher() = default;
		virtual ~Renderer3DFlusher() = default;
		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Flush() = 0;
		virtual void PushCubeVertex(CubeVertex* begin,CubeVertex* end) = 0;
	};

	

}
