
/***********************************************************************

								h��SweepingTrail

	A Sweeping trail several main characteristic:
	1. a trail path is driven by a sequence of spatial line segments
	2. adjacent line segments pairs can build up 2 triangles ( or a spatial 'quad')
	3. the header of the trail (the head line segment) must be updated in every frame
	4. the header of the trail should be "cooled down" when reached a given time duration limit.
			A new "free" header line trail shall be generated by then.
	5. the tail of the trail(the last line segment) must be updated in every frame, and
			approaching to the second last line segment over time to reduce the area 
			of the last quad (Meanwhile, the u coord of the tail vertices is maintained to 1)
	6. (2018.7.23)the headers' vertices u coord should be 0, while the tail should be 1.


	vector of line segment (vector index 0-->n, line/vertex index 0-->n)
	v_0	-----	 v_2	......	-----		v_n-3	------	v_n-1
	  |											  |						  |
	v_1 -----	v_3  ......	-----		v_n-2	------	v_n-0(free tail)(approaching the 2nd last LS)
	(free header)							

	sweeping trail moving direction
	<---------------------

************************************************************************/

#include "Noise3D.h"
#include "SweepingTrail.h"

using namespace Noise3D;

Noise3D::ISweepingTrail::ISweepingTrail() :
	mInterpolationStepCount(1),
	mLastDrawnVerticesCount(0),
	mCubicHermiteTangentScale(0.5f),
	mGpuVertexPoolByteCapacity(0),
	mGpuPoolMaxVertexCount(0),
	mHeaderCoolDownTimer(0.0f),
	mHeaderCoolDownTimeThreshold(20.0f),
	mMaxLifeTimeOfLS(1000.0f),
	mFreeTailTangent1(233.0f,666.0f,666.0f),
	mFreeTailTangent2(233.0f, 666.0f, 666.0f)
{
}

Noise3D::ISweepingTrail::~ISweepingTrail()
{
	ReleaseCOM(m_pVB_Gpu);
}

void Noise3D::ISweepingTrail::SetHeader(N_LineSegment lineSeg)
{
	mFreeHeader = lineSeg;
	//if (!mIsHeaderActive)mIsHeaderActive = true;
}

N_LineSegment Noise3D::ISweepingTrail::GetHeader()
{
	return mFreeHeader;
}

NVECTOR3 Noise3D::ISweepingTrail::GetHeaderCenterPos()
{
	return (mFreeHeader.vert1+mFreeHeader.vert2)/2.0f;
}

void Noise3D::ISweepingTrail::SetHeaderCoolDownTimeThreshold(float duration)
{
	mHeaderCoolDownTimeThreshold = duration;
}

void Noise3D::ISweepingTrail::SetMaxLifeTimeOfLineSegment(float duration)
{
	mMaxLifeTimeOfLS = duration;
}

uint32_t Noise3D::ISweepingTrail::GetLastDrawnVerticesCount()
{
	return mLastDrawnVerticesCount ;
}

void Noise3D::ISweepingTrail::SetInterpolationStepCount(uint32_t count)
{
	if (count == 0)count = 1;
	mInterpolationStepCount = count;
}

void Noise3D::ISweepingTrail::SetCubicHermiteTangentScale(float length)
{
	mCubicHermiteTangentScale = length;
}

void Noise3D::ISweepingTrail::Update(float deltaTime)
{
	//timer add
	mHeaderCoolDownTimer += deltaTime;

	//ensure that there is at least one cooled down line segment
	if (mFixedLineSegments.empty())
	{
		mFixedLineSegments.push_back(mFreeHeader);
		//maybe it's in initial state, reset the tail
		mFreeTail_Start = mFreeHeader;
		mFreeTailTangent1 = NVECTOR3(0, 0, 0);
		mFreeTailTangent2 = NVECTOR3(0, 0, 0);
	}

	mFunction_CoolDownHeader();
	mFunction_MoveAndCollapseTail();
	mFunction_GenVerticesAndUpdateToGpuBuffer();
}

bool Noise3D::ISweepingTrail::IsRenderable()
{
	return !mFixedLineSegments.empty();//at least a quad(emmm, ideally it should have at least 2 quads)
}

void Noise3D::ISweepingTrail::GetTangentList(std::vector<std::pair<NVECTOR3, NVECTOR3>>& outList)
{
	//copy the tangent list outside for debug use
	outList = mTangentList;
}

void Noise3D::ISweepingTrail::GetVerticesList(std::vector<Noise3D::N_LineSegment>& outList)
{
	outList = mFixedLineSegments;
	outList.insert(outList.begin(), mFreeHeader);
	outList.push_back(mFreeTail_Start);
}

/*****************************************************************
											PRIVATE
*****************************************************************/
bool NOISE_MACRO_FUNCTION_EXTERN_CALL Noise3D::ISweepingTrail::mFunction_InitGpuBuffer(UINT maxVertexCount)
{
	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex, sizeof(tmpInitData_Vertex));
	std::vector<N_SweepingTrailVertexType> tmpVec(maxVertexCount);
	tmpInitData_Vertex.pSysMem = &tmpVec.at(0);
	mGpuVertexPoolByteCapacity = sizeof(N_SweepingTrailVertexType)* maxVertexCount;
	mGpuPoolMaxVertexCount = maxVertexCount;

	//Simple Vertex!
	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth = mGpuVertexPoolByteCapacity;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	//create gpu vertex buffer
	int hr = 0;
	hr = D3D::g_pd3dDevice11->CreateBuffer(&vbd, &tmpInitData_Vertex, &m_pVB_Gpu);
	HR_DEBUG(hr, "SweepingTrail : Failed to create vertex pool !");

	return true;
}

void Noise3D::ISweepingTrail::mFunction_CoolDownHeader()
{
	//if fixed line segment exist
	if (mFixedLineSegments.size() > 0)
	{
		//cool down current header, and GENERATE a NEW free segment (add to front)
		//note that when a new line segment cool down, we "add front"
		//thus vector.front() is right after the header of the line sequence
		if (mHeaderCoolDownTimer >= mHeaderCoolDownTimeThreshold)
		{
			mFixedLineSegments.insert(mFixedLineSegments.begin(), mFreeHeader);
			mHeaderCoolDownTimer = 0.0f;
		}
	}
}

void Noise3D::ISweepingTrail::mFunction_MoveAndCollapseTail()
{
	//if at least one fixed line segment exists
	if (mFixedLineSegments.size() > 0)
	{

		//the tail keeps moving to the second last line segment. when the last LS reached the second last,
		//the previous last LS can be removed (thus previous last quad has DEGENERATE into a LS)
		//!!!!:An important point: the collapsing time of the last quad == the header cool down threshold
		//!!!because every line segment has their own 'Life Time'. When a line segment is cooled down, its own
		//life timer start from 0 and tick.
		//And now i want ensure that the last LS's texcoord u maintain 1.0f(while it life timer is exactly equals to 'MaxLifeTime'
		//so the last LS must move to adapt, the lerp ratio is computed below
		float tailLSLifeTimer = mFunction_UtComputeLSLifeTimer(mFixedLineSegments.size() + 2 - 1);
		mTailQuadCollapsingRatio = (tailLSLifeTimer - mMaxLifeTimeOfLS) / mHeaderCoolDownTimeThreshold;
		mTailQuadCollapsingRatio = Ut::Clamp(mTailQuadCollapsingRatio, 0.0f, 1.0f);
		//mFreeTail_Current.vert1 = Ut::Lerp(mFreeTail_Start.vert1, mFixedLineSegments.back().vert1, mTailQuadCollapsingRatio);
		//mFreeTail_Current.vert2 = Ut::Lerp(mFreeTail_Start.vert2, mFixedLineSegments.back().vert2, mTailQuadCollapsingRatio);

		//collapse last quad & degenerate
		if (mTailQuadCollapsingRatio >= 1.0f)
		{
			mFreeTail_Start = mFixedLineSegments.back();

			//store the 2nd last tangents to new free tail tangent
			mFunction_UtGetTangent(mFixedLineSegments.size()-1, mFreeTailTangent1, mFreeTailTangent2);
			//mFreeTailTangent1 = mTangentList.at(mTangentList.size()-2).first;
			//mFreeTailTangent2 = mTangentList.at(mTangentList.size() - 2).second;
			mFixedLineSegments.pop_back();//pop the last fixed line from the vector's back
			mTailQuadCollapsingRatio = 0.0f;
		}
	}
}

void Noise3D::ISweepingTrail::mFunction_GenVerticesAndUpdateToGpuBuffer()
{
	mFunction_UtEstimateTangents();

	if (mFixedLineSegments.size() > 0)
	{
		//****Map*****
		//(2018.7.24)if vertex size in memory exceeds GPU vertex pool capacity, then some of the front vertices won't be uploaded.
		//pre-caution for index-out-of-range situation
		int maxStartVertexIndex = mGpuPoolMaxVertexCount - mInterpolationStepCount * 6;

		D3D11_MAPPED_SUBRESOURCE mappedRes;
		HRESULT hr = D3D::g_pImmediateContext->Map(m_pVB_Gpu, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedRes);
		if (FAILED(hr))
		{
			ERROR_MSG("SweepingTrail: Failed to update vertices.");
			return;
		}

		int vertexIndexOffset = 0;
		int totalRegionCount = mFixedLineSegments.size() + 1;
		for (int i = 0; i < totalRegionCount; ++i)//be careful of corner cases
		{
			if (vertexIndexOffset > maxStartVertexIndex)
			{
				return;//not enought mem space, exit
			}
			N_SweepingTrailVertexType* tmpMemAddr = (N_SweepingTrailVertexType*)mappedRes.pData + vertexIndexOffset;
			N_GenQuadInfo tmpInfo;
			N_LineSegment frontLS = mFunction_UtGetLineSegment(i);
			N_LineSegment backLS = mFunction_UtGetLineSegment(i + 1);
			tmpInfo.interpolation_steps = mInterpolationStepCount;
			tmpInfo.frontPos1 = frontLS.vert1;
			tmpInfo.frontPos2 = frontLS.vert2;
			tmpInfo.backPos1 = backLS.vert1;
			tmpInfo.backPos2 = backLS.vert2;
			//uses adjacent vertices to estimate current vertex's tangent, CORNER CASES are dealt with inside the function
			tmpInfo.frontTangent1 = mTangentList.at(i).first;
			tmpInfo.frontTangent2 = mTangentList.at(i).second;
			tmpInfo.backTangent1 = mTangentList.at(i+1).first;
			tmpInfo.backTangent2 = mTangentList.at(i+1).second;
			//mFunction_UtGetTangent(i, tmpInfo.frontTangent1, tmpInfo.frontTangent2);
			//mFunction_UtGetTangent(i + 1, tmpInfo.backTangent1, tmpInfo.backTangent2);

			//interpolation of the last region is dealt with differently (to ensure that the tail collapses on history path)
			if (i == totalRegionCount - 1)
			{
				tmpInfo.collapsingFactor = mTailQuadCollapsingRatio;
			}
			//Gen normal interpolated Quads
			vertexIndexOffset += mFunction_UtGenQuad(tmpInfo, mFunction_UtComputeLSLifeTimer(i), mFunction_UtComputeLSLifeTimer(i + 1), tmpMemAddr);

			//**********Unmap***************
			D3D::g_pImmediateContext->Unmap(m_pVB_Gpu, 0);
			mLastDrawnVerticesCount = vertexIndexOffset;
		}
	}

}
//compute current life time elapsed of line segment in given position
float Noise3D::ISweepingTrail::mFunction_UtComputeLSLifeTimer(int index)
{
	//index==0, free header						|
	//index==1, free header+1 fixed LS   |--|
	//index==2, free header+2 fixed LS	|--|-----|
	//index==3, free header+3 fixed LS	|--|-----|-----|
	if (index == 0)return 0.0f;
	return (mHeaderCoolDownTimer + (index-1) * mHeaderCoolDownTimeThreshold);
}

//gen quad to given mem position (6 vertices occupied)
int Noise3D::ISweepingTrail::mFunction_UtGenQuad(const N_GenQuadInfo& desc, float frontLifeTimer, float backLifeTimer, N_SweepingTrailVertexType* quad)
{
	/*
	vector of line segment (vector index 0-->n, line/vertex index 0-->n)
	v_0	-----	 v_2	......	-----		v_n-3	------	v_n-1
	  |											  |						  |
	v_1 -----	v_3  ......	-----		v_n-2	------	v_n-0(free tail)(approaching the 2nd last LS)
	(free header)							

	sweeping trail moving direction
	<---------------------
	*/

	//Cubic Hermite interplation is used between each passed-in pair of line segments
	float unitRatio = 1.0f / desc.interpolation_steps;
	for (int i = 0; i < desc.interpolation_steps; ++i)
	{
		//tailCollapsingFactor is usually 0.0f (but the tail LS's factor will be larger than 0.0f because the tail keep approaching the second last LS)
		float frontLerpRatio = (1.0f - desc.collapsingFactor) *  i * unitRatio;
		float backLerpRatio = (1.0f - desc.collapsingFactor) * (i + 1) * unitRatio;

		//front line segment in interpolaton(start and end line segment is passed in as function param)
		NVECTOR3 interpFrontPos1	=	Ut::CubicHermite(desc.frontPos1, desc.backPos1, desc.frontTangent1 ,desc.backTangent1 , frontLerpRatio);
		NVECTOR3 interpFrontPos2	=	Ut::CubicHermite(desc.frontPos2, desc.backPos2, desc.frontTangent2, desc.backTangent2, frontLerpRatio);
		NVECTOR3 interpBackPos1		=	Ut::CubicHermite(desc.frontPos1, desc.backPos1, desc.frontTangent1, desc.backTangent1, backLerpRatio);
		NVECTOR3 interpBackPos2		=	Ut::CubicHermite(desc.frontPos2, desc.backPos2, desc.frontTangent2, desc.backTangent2, backLerpRatio);
		//Position
		//021
		quad[i * 6 + 0].Pos = interpFrontPos1;
		quad[i * 6 + 1].Pos = interpBackPos1;
		quad[i * 6 + 2].Pos = interpFrontPos2;

		//123
		quad[i * 6 + 3].Pos = interpFrontPos2;
		quad[i * 6 + 4].Pos = interpBackPos1;
		quad[i * 6 + 5].Pos = interpBackPos2;

		//UV, all texcoord.u decreases in a constant rate
		//ensure that the tail line segment' texcoord.u is 0, header is 1
		float frontTexcoordU = Ut::Lerp(frontLifeTimer / mMaxLifeTimeOfLS, backLifeTimer / mMaxLifeTimeOfLS, frontLerpRatio);
		float backTexcoordU = Ut::Lerp(frontLifeTimer / mMaxLifeTimeOfLS, backLifeTimer / mMaxLifeTimeOfLS, backLerpRatio);

		quad[i * 6 + 0].TexCoord = NVECTOR2(frontTexcoordU, 0.0f);
		quad[i * 6 + 1].TexCoord = NVECTOR2(backTexcoordU, 0.0f);
		quad[i * 6 + 2].TexCoord = NVECTOR2(frontTexcoordU, 1.0f);

		quad[i * 6 + 3].TexCoord = NVECTOR2(frontTexcoordU, 1.0f);
		quad[i * 6 + 4].TexCoord = NVECTOR2(backTexcoordU, 0.0f);
		quad[i * 6 + 5].TexCoord = NVECTOR2(backTexcoordU, 1.0f);

		quad[i * 6 + 0].Color = NVECTOR4(quad[i * 6 + 0].TexCoord.x, quad[i * 6 + 0].TexCoord.y, 0.0f, 1.0f);
		quad[i * 6 + 1].Color = NVECTOR4(quad[i * 6 + 1].TexCoord.x, quad[i * 6 + 1].TexCoord.y, 0.0f, 1.0f);
		quad[i * 6 + 2].Color = NVECTOR4(quad[i * 6 + 2].TexCoord.x, quad[i * 6 + 2].TexCoord.y, 0.0f, 1.0f);

		quad[i * 6 + 3].Color = NVECTOR4(quad[i * 6 + 3].TexCoord.x, quad[i * 6 + 3].TexCoord.y, 0.0f, 1.0f);
		quad[i * 6 + 4].Color = NVECTOR4(quad[i * 6 + 4].TexCoord.x, quad[i * 6 + 4].TexCoord.y, 0.0f, 1.0f);
		quad[i * 6 + 5].Color = NVECTOR4(quad[i * 6 + 5].TexCoord.x, quad[i * 6 + 5].TexCoord.y, 0.0f, 1.0f);

	}

	//vertices generated
	//2 line segment, 2 triangles, 6 vertices
	const int c_quadVertexCount = 6;
	return (desc.interpolation_steps) * c_quadVertexCount;
}

void Noise3D::ISweepingTrail::mFunction_UtEstimateTangents()
{
	mTangentList.resize(mFixedLineSegments.size() + 2);
	mTangentList.at(0).first = mFixedLineSegments.front().vert1 - mFreeHeader.vert1;
	mTangentList.at(0).first *= mCubicHermiteTangentScale;
	mTangentList.at(0).second = mFixedLineSegments.front().vert2 - mFreeHeader.vert2;
	mTangentList.at(0).second *= mCubicHermiteTangentScale;

	for (int i = 1; i <= mFixedLineSegments.size(); ++i)
	{
		N_LineSegment frontLS = mFunction_UtGetLineSegment(i - 1);
		N_LineSegment backLS = mFunction_UtGetLineSegment(i + 1);
		mTangentList.at(i).first = backLS.vert1 - frontLS.vert1;
		mTangentList.at(i).first *= mCubicHermiteTangentScale;
		mTangentList.at(i).second = backLS.vert2 - frontLS.vert2;
		mTangentList.at(i).second *= mCubicHermiteTangentScale;
	}

	mTangentList.at(mFixedLineSegments.size() + 1).first = mFreeTailTangent1;
	mTangentList.at(mFixedLineSegments.size() + 1).second = mFreeTailTangent2;
}

void Noise3D::ISweepingTrail::mFunction_UtGetTangent(int currentLineSegmentIndex, NVECTOR3 & outTangent1, NVECTOR3 & outTangent2)
{
	outTangent1 = mTangentList.at(currentLineSegmentIndex).first;
	outTangent2 = mTangentList.at(currentLineSegmentIndex).second;
}

inline N_LineSegment Noise3D::ISweepingTrail::mFunction_UtGetLineSegment(int index)
{
	//header
	if (index == 0)return mFreeHeader;
	//tail
	if (index == mFixedLineSegments.size() + 2 - 1)return mFreeTail_Start;
	//middle 
	if (mFixedLineSegments.size() > 0)return mFixedLineSegments.at(index - 1);
	//it shouldn't run to here
	return mFreeHeader;
}
