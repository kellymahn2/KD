#include "KDpch.h"
#include "Entity.h"

namespace Kaidel {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

}