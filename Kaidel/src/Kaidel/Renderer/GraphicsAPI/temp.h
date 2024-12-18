#pragma once
#define TPACK(...) __VA_ARGS__
#define TMAKE(ret, name, inputs, vulkan, inputNames)\
ret name##(inputs){\
	switch (Renderer::GetAPI())\
	{\
	case RendererAPI::API::Vulkan: return vulkan##(inputNames);\
	}\
	\
	return {};\
}
