#pragma once

using namespace Noise3D;

class SceneLoader
{
public:

	void Init(SceneManager* pMgr);

	void LoadScene();

private:

	void _LoadTextures();

	void _LoadMaterials();

	void _LoadSphere(SceneGraph& sg,Vec3 pos, float radius);

	void _LoadBox(SceneGraph& sg,Vec3 pos, Vec3 size);

	void _LoadRect(SceneGraph& sg,NOISE_RECT_ORIENTATION ori, Vec3 pos, Vec2 size);

protected:

	SceneManager* m_pScene;
	MeshManager* m_pMeshMgr;
	MeshLoader* m_pModelLoader;
	MaterialManager*	m_pMatMgr;
	TextureManager*	m_pTexMgr;
	LogicalShapeManager* m_pShapeMgr;
	std::vector<Mesh*> mRealTimeRenderMeshList;

	GI::AdvancedGiMaterial* m_pMat1;
};