#include "Noise3D.h"
#include <sstream>

BOOL Init3D(HWND hwnd);
void MainLoop();
void Cleanup();
void	InputProcess();

using namespace Noise3D;

IRoot* pRoot;
IRenderer* pRenderer;
IScene* pScene;
ICamera* pCamera;
IAtmosphere* pAtmos;
IModelLoader* pModelLoader;
IMeshManager* pMeshMgr;
std::vector<IMesh*> meshList;
IMaterialManager*	pMatMgr;
ITextureManager*	pTexMgr;
IGraphicObjectManager*	pGraphicObjMgr;
IGraphicObject*	pGraphicObjBuffer;
ISweepingTrailManager* pSweepingTrailMgr;
ISweepingTrail* pSweepingTrail;
ILightManager* pLightMgr;
IDirLightD*		pDirLight1;
IPointLightD*	pPointLight1;
ISpotLightD*	pSpotLight1;
IFontManager* pFontMgr;
IDynamicText* pMyText_fps;


Ut::ITimer timer(Ut::NOISE_TIMER_TIMEUNIT_MILLISECOND);
Ut::IInputEngine inputE;

//Main Entry
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{

	//get Root interface
	pRoot = GetRoot();

	//create a window (using default window creation function)
	HWND windowHWND;
	windowHWND = pRoot->CreateRenderWindow(1280, 800, L"Hahaha Render Window", hInstance);

	//initialize input engine (detection for keyboard and mouse input)
	inputE.Initialize(hInstance, windowHWND);

	//D3D and scene object init
	Init3D(windowHWND);

	//register main loop function
	pRoot->SetMainLoopFunction(MainLoop);

	//enter main loop
	pRoot->Mainloop();

	//program end
	Cleanup();

	//..
	return 0;
}

BOOL Init3D(HWND hwnd)
{
	const UINT bufferWidth = 1280;
	const UINT bufferHeight = 800;

	//init fail
	if (!pRoot->Init())return FALSE;

	//query pointer to IScene
	pScene = pRoot->GetScenePtr();

	pMeshMgr = pScene->GetMeshMgr();
	pRenderer = pScene->CreateRenderer(bufferWidth, bufferHeight, hwnd);
	pCamera = pScene->GetCamera();
	pLightMgr = pScene->GetLightMgr();
	pMatMgr = pScene->GetMaterialMgr();
	pTexMgr = pScene->GetTextureMgr();
	pAtmos = pScene->GetAtmosphere();
	pGraphicObjMgr = pScene->GetGraphicObjMgr();
	pSweepingTrailMgr = pScene->GetSweepingTraillMgr();

	pTexMgr->CreateTextureFromFile("../media/earth.jpg", "Earth", TRUE, 1024, 1024, FALSE);
	pTexMgr->CreateTextureFromFile("../media/universe.jpg", "Universe", FALSE, 256, 256, FALSE);
	ITexture* pNormalMap = pTexMgr->CreateTextureFromFile("../media/earth-normal.png", "EarthNormalMap", FALSE, 512, 512, TRUE);

	//create font texture
	pFontMgr = pScene->GetFontMgr();
	pFontMgr->CreateFontFromFile("../media/calibri.ttf", "myFont", 24);
	pMyText_fps = pFontMgr->CreateDynamicTextA("myFont", "fpsLabel", "fps:000", 200, 100, NVECTOR4(0, 0, 0, 1.0f), 0, 0);
	pMyText_fps->SetTextColor(NVECTOR4(0, 0.3f, 1.0f, 0.5f));
	pMyText_fps->SetDiagonal(NVECTOR2(20, 20), NVECTOR2(300, 60));
	pMyText_fps->SetFont("myFont");
	pMyText_fps->SetBlendMode(NOISE_BLENDMODE_ALPHA);

	//------------------MESH INITIALIZATION----------------
	//pModelLoader->LoadSphere(pMesh1,5.0f, 30, 30);
	pModelLoader = pScene->GetModelLoader();
	N_SceneLoadingResult res;
	//pModelLoader->LoadFile_FBX("../model/geoScene-fbx/geometries2.FBX", res);
	//pModelLoader->LoadFile_FBX("../media/model/teapot.fbx", res);
	IMesh* pMesh = pMeshMgr->CreateMesh("testModel");
	pModelLoader->LoadSphere(pMesh, 20.0f, 20, 20);
	pMesh->SetPosition(0, 0, 0);
	pMesh->SetCullMode(NOISE_CULLMODE_NONE);
	pMesh->SetShadeMode(NOISE_SHADEMODE_GOURAUD);
	pMesh->SetShadeMode(NOISE_SHADEMODE_PHONG);
	meshList.push_back(pMesh);
	/*for (auto & name : res.meshNameList)
	{
	IMesh* pMesh = pMeshMgr->GetMesh(name);
	meshList.push_back(pMesh);
	pMesh->SetCullMode(NOISE_CULLMODE_BACK);
	pMesh->SetShadeMode(NOISE_SHADEMODE_GOURAUD);
	}*/

	const std::vector<N_DefaultVertex>* pTmpVB;
	pTmpVB = meshList.at(0)->GetVertexBuffer();
	pGraphicObjBuffer = pGraphicObjMgr->CreateGraphicObj("normalANDTangent");
	NVECTOR3 modelPos = meshList.at(0)->GetPosition();
	for (auto v : *pTmpVB)
	{
		//pGraphicObjBuffer->AddLine3D(modelPos + v.Pos, modelPos+ v.Pos + 5.0f * v.Normal, NVECTOR4(1.0f, 0, 0, 1.0f), NVECTOR4(0,0,0, 1.0f));//draw the normal
		//pGraphicObjBuffer->AddLine3D(modelPos + v.Pos, modelPos + v.Pos + 5.0f* v.Tangent, NVECTOR4(0,0, 1.0f, 1.0f), NVECTOR4(1.0f,1.0f,1.0f, 1.0f));//draw the tangent
	}
	pGraphicObjBuffer->AddLine3D({ 0,0,0 }, { 50.0f,0,0 }, { 1.0f,0,0,1.0f }, { 1.0f,0,0,1.0f });
	pGraphicObjBuffer->AddLine3D({ 0,0,0 }, { 0,50.0f,0 }, { 0,1.0f,0,1.0f }, { 0,1.0f,0,1.0f });
	pGraphicObjBuffer->AddLine3D({ 0,0,0 }, { 0,0,50.0f }, { 0,0,1.0f,1.0f }, { 0,0,1.0f,1.0f });

	//----------------------------------------------------------

	pCamera->SetPosition(0, 30.0f, 0);
	pCamera->SetLookAt(0, 0, 0);
	pCamera->SetViewAngle_Radian(MATH_PI / 2.5f, 1.333333333f);
	pCamera->SetViewFrustumPlane(1.0f, 500.f);


	pModelLoader->LoadSkyDome(pAtmos, "Universe", 4.0f, 2.0f);
	pAtmos->SetFogEnabled(false);
	pAtmos->SetFogParameter(50.0f, 100.0f, NVECTOR3(0, 0, 1.0f));

	//---------------Light-----------------
	pDirLight1 = pLightMgr->CreateDynamicDirLight("myDirLight1");
	N_DirLightDesc dirLightDesc;
	dirLightDesc.ambientColor = NVECTOR3(0.1f, 0.1f, 0.1f);
	dirLightDesc.diffuseColor = NVECTOR3(1.0f, 1.0f, 1.0f);
	dirLightDesc.specularColor = NVECTOR3(1.0f, 1.0f, 1.0f);
	dirLightDesc.direction = NVECTOR3(1.0f, -1.0f, 0);
	dirLightDesc.specularIntensity = 1.0f;
	dirLightDesc.diffuseIntensity = 1.0f;
	pDirLight1->SetDesc(dirLightDesc);

	//bottom right
	pGraphicObjBuffer->AddRectangle(NVECTOR2(1080.0f, 780.0f), NVECTOR2(1280.0f,800.0f), NVECTOR4(0.3f, 0.3f, 1.0f, 1.0f), "BottomRightTitle");
	pGraphicObjBuffer->SetBlendMode(NOISE_BLENDMODE_ALPHA);

	//*********************  sweeping trail  *************************
	pSweepingTrail = pSweepingTrailMgr->CreateSweepingTrail("myFX_Trail", 1000);
	pSweepingTrail->SetBlendMode(NOISE_BLENDMODE_ALPHA);
	pSweepingTrail->SetFillMode(NOISE_FILLMODE_WIREFRAME);
	pSweepingTrail->SetHeaderCoolDownTimeThreshold(50.0f);
	pSweepingTrail->SetMaxLifeTimeOfLineSegment(200.0f);
	pSweepingTrail->SetHeader(N_LineSegment(NVECTOR3(0.0f, -10.0f, 0.0f), NVECTOR3(0.0, 10.0f, 0.0f)));
	pSweepingTrail->SetInterpolationStepCount(10);
	pSweepingTrail->SetCubicHermiteTangentScale(0.7f);

	return TRUE;
};

void MainLoop()
{
	static float incrNum = 0.0;
	incrNum += 0.2f;
	::Sleep(100);

	InputProcess();
	pRenderer->ClearBackground(NVECTOR4(0.7f,0.7f,0.7f,1.0f));
	timer.NextTick();

	//update fps lable
	std::stringstream tmpS;
	tmpS << "fps :" << timer.GetFPS() << "__vertex count:" << pSweepingTrail->GetLastDrawnVerticesCount();// << std::endl;
	pMyText_fps->SetTextAscii(tmpS.str());

	pSweepingTrail->SetHeader(N_LineSegment(NVECTOR3(10.0f*sinf(incrNum), 0, 10.0f*cosf(incrNum)), NVECTOR3(5.0f*sinf(incrNum), 0, 5.0f*cosf(incrNum))));
	pSweepingTrail->Update(10.0f);

	//add to render list
	//for (auto& pMesh : meshList)pRenderer->AddToRenderQueue(pMesh);
	pRenderer->AddToRenderQueue(pGraphicObjBuffer);
	pRenderer->AddToRenderQueue(pMyText_fps);
	pRenderer->AddToRenderQueue(pSweepingTrail);
	//pRenderer->SetActiveAtmosphere(pAtmos);
	static N_PostProcessGreyScaleDesc desc;
	desc.factorR = 0.3f;
	desc.factorG = 0.59f;
	desc.factorB = 0.11f;
	//pRenderer->AddToPostProcessList_GreyScale(desc);

	//render
	pRenderer->Render();

	//present
	pRenderer->PresentToScreen();
};

void InputProcess()
{
	inputE.Update();

	if (inputE.IsKeyPressed(Ut::NOISE_KEY_A))
	{
		pCamera->fps_MoveRight(-0.02f * timer.GetInterval() , FALSE);
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_D))
	{
		pCamera->fps_MoveRight(0.02f * timer.GetInterval(), FALSE);
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_W))
	{
		pCamera->fps_MoveForward(0.02f* timer.GetInterval(), FALSE);
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_S))
	{
		pCamera->fps_MoveForward(-0.02f* timer.GetInterval(), FALSE);
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_SPACE))
	{
		pCamera->fps_MoveUp(0.02f* timer.GetInterval());
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_LCONTROL))
	{
		pCamera->fps_MoveUp(-0.02f* timer.GetInterval());
	}

	//Sweeping Trail movement
	/*if (inputE.IsKeyPressed(Ut::NOISE_KEY_J))
	{
		 N_LineSegment ls = pSweepingTrail->GetHeader();
		pSweepingTrail->SetHeader(N_LineSegment( ls.vert1 +NVECTOR3(0.1f,0.0f,0.0f), ls.vert2+NVECTOR3(0.1f, 0.0f, 0.0f)));
	}
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_K))
	{
		N_LineSegment ls = pSweepingTrail->GetHeader();
		pSweepingTrail->SetHeader(N_LineSegment(ls.vert1 + NVECTOR3(-0.1f, -0.0f, 0.0f), ls.vert2 + NVECTOR3(-0.1f, -0.0f, 0.0f)));
	}*/


	if (inputE.IsMouseButtonPressed(Ut::NOISE_MOUSEBUTTON_LEFT))
	{
		pCamera->RotateY_Yaw((float)inputE.GetMouseDiffX() / 200.0f);
		pCamera->RotateX_Pitch((float)inputE.GetMouseDiffY() / 200.0f);
	}

	//quit main loop and terminate program
	if (inputE.IsKeyPressed(Ut::NOISE_KEY_ESCAPE))
	{
		pRoot->SetMainLoopStatus(NOISE_MAINLOOP_STATUS_QUIT_LOOP);
	}

};

void Cleanup()
{
	pRoot->ReleaseAll();
};
