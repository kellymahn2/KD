#include <Kaidel.h>
#include <Kaidel/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Kaidel {

	class Kaidelnut : public Application
	{
	public:
		Kaidelnut()
			: Application("Kaidelnut")
		{
			PushLayer(new EditorLayer());
		}

		~Kaidelnut()
		{
		}
	};

	Application* CreateApplication()
	{
		return new Kaidelnut();
	}

}