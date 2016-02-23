#include "ToyGE\RenderEngine\StaticMeshActor.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\Kernel\MeshAsset.h"

namespace ToyGE
{
	Ptr<StaticMeshActor> StaticMeshActor::Create(const Ptr<Scene> & scene, const Ptr<class Mesh> & mesh)
	{
		auto meshCom = std::make_shared<RenderMeshComponent>();
		meshCom->SetMesh(mesh);
		auto actor = std::make_shared<StaticMeshActor>();
		scene->AddActor(actor);
		actor->AddComponent(meshCom);
		actor->ActivateAllComponents();
		return actor;
	}

	Ptr<StaticMeshActor> StaticMeshActor::Create(const Ptr<Scene> & scene, const String & meshPath)
	{
		return Create(scene, Asset::FindAndInit<MeshAsset>(meshPath)->GetMesh());
	}
}