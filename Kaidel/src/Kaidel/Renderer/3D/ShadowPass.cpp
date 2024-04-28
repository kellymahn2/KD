#include "KDpch.h"
#include "Kaidel/Renderer/3D/Light.h"
#include "ShadowPass.h"
#include "Kaidel/Scene/Components.h"
#include "BeginPass.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Core/JobSystem.h"
namespace Kaidel {

	static std::mutex s_RenderingMutex;
}
