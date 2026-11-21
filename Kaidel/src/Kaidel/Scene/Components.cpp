#include "KDpch.h"

#include "Entity.h"
#include "Components.h"


namespace Kaidel
{
	void SkinnedMeshComponent::SetRootBone(Entity& entity)
	{
		if (auto root = entity.GetScene()->GetEntity(RootBone); root && root.HasComponent<RootBoneComponent>())
		{
			if (--root.GetComponent<RootBoneComponent>().UsageCount == 0)
			{
				root.RemoveComponent<RootBoneComponent>();
			}
		}

		++entity.TryAddComponent<RootBoneComponent>().UsageCount;

		RootBone = entity.GetUUID();
	}

}
