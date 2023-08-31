#include <Kaidel.h>
#include <Kaidel/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Kaidel {

	class Kaidelnut : public Application
	{
	public:
		Kaidelnut(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new EditorLayer());
		}

		~Kaidelnut()
		{
		}
	};

	Application* CreateApplication(const ApplicationCommandLineArgs& args)
	{
		ApplicationSpecification spec;
		spec.Name = "Editor";
		spec.WorkingDirectory = "../KaidelEditor";
		spec.CommandLineArgs = args;
		return new Kaidelnut(spec);
	}
}
