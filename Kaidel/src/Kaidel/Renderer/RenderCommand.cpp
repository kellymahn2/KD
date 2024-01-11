#include "KDpch.h"
#include "Kaidel/Renderer/RenderCommand.h"

namespace Kaidel {

	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

}
