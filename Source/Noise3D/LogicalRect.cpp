
/*******************************************************

								cpp ��Logical Rect

********************************************************/

#include "Noise3D.h"
#include "Noise3D_InDevHeader.h"

using namespace Noise3D;

Noise3D::LogicalRect::LogicalRect():
	mOrientation(NOISE_RECT_ORIENTATION::RECT_XZ),
	mSize(1.0f,1.0f)
{
}

Noise3D::LogicalRect::~LogicalRect()
{
}

void Noise3D::LogicalRect::SetOrientation(NOISE_RECT_ORIENTATION ori)
{
	mOrientation = ori;
}

NOISE_RECT_ORIENTATION Noise3D::LogicalRect::GetOrientation() const
{
	return mOrientation;
}

Vec3 Noise3D::LogicalRect::ComputeNormal()
{
	switch (mOrientation)
	{
	case NOISE_RECT_ORIENTATION::RECT_XY:
		return Vec3(0, 0, 1.0f);
		break;

	case NOISE_RECT_ORIENTATION::RECT_XZ:
		return Vec3(0, 1.0f, 0);
		break;

	case NOISE_RECT_ORIENTATION::RECT_YZ:
		return Vec3(1.0f, 0, 0);
		break;
	}

	return Vec3(0, 0, 0);
}

Vec2 Noise3D::LogicalRect::ComputeUV(Vec3 pos)
{
	Vec2 negHalfSize = -mSize / 2.0f;
	float u = 0.0f, v = 0.0f;

	switch (mOrientation)
	{
	case NOISE_RECT_ORIENTATION::RECT_XY:
		u = (pos.x - negHalfSize.x) / mSize.x;
		v = (pos.y - negHalfSize.y) / mSize.y;
		break;

	case NOISE_RECT_ORIENTATION::RECT_XZ:
		u = (pos.x - negHalfSize.x) / mSize.x;
		v = (pos.z - negHalfSize.y) / mSize.y;
		break;

	case NOISE_RECT_ORIENTATION::RECT_YZ:
		u = (pos.y - negHalfSize.x) / mSize.x;
		v = (pos.z - negHalfSize.y) / mSize.y;
		break;
	}

	u = std::fmodf(u, 1.0f);
	v = std::fmodf(v, 1.0f);
	if (mUVNegative)
	{
		u = 1.0f - u;
		v = 1.0f - v;
	}
	return Vec2(u, v);
}

void Noise3D::LogicalRect::SetSize(float width, float height)
{
	if (width != 0 && height != 0)
	{
		mSize.x = width;
		mSize.y = height;
	}
}

void Noise3D::LogicalRect::SetSize(Vec2 size)
{
	if (size.x != 0 && size.y != 0)
	{
		mSize = size;
	}
}

Vec2 Noise3D::LogicalRect::GetSize() const
{
	return mSize;
}

Vec3 Noise3D::LogicalRect::GenLocalRandomPoint()
{
	GI::RandomSampleGenerator g;
	switch (mOrientation)
	{
	case NOISE_RECT_ORIENTATION::RECT_XY:
		return Vec3(g.NormalizedReal() * mSize.x/2.0f, g.NormalizedReal() * mSize.y /2.0f, 0.0f);
		break;

	case NOISE_RECT_ORIENTATION::RECT_XZ:
		return Vec3(g.NormalizedReal() * mSize.x / 2.0f,  0.0f, g.NormalizedReal() * mSize.y / 2.0f);
		break;

	case NOISE_RECT_ORIENTATION::RECT_YZ:
		return Vec3(0.0f, g.NormalizedReal() * mSize.x / 2.0f, g.NormalizedReal() * mSize.y / 2.0f);
		break;
	}
	return Vec3(0, 0, 0);
}

void Noise3D::LogicalRect::SetUVNegative(bool isNeg)
{
	mUVNegative = isNeg;
}

NOISE_SCENE_OBJECT_TYPE Noise3D::LogicalRect::GetObjectType() const
{
	return NOISE_SCENE_OBJECT_TYPE::LOGICAL_RECT;
}

N_AABB Noise3D::LogicalRect::GetLocalAABB()
{
	N_AABB res;
	Vec3 halfSize = mSize / 2.0f;
	const float epsilon = 1e-4f;//epsilon thickness
	switch (mOrientation)
	{
	case NOISE_RECT_ORIENTATION::RECT_XY:
		res.min = Vec3(-halfSize.x, -halfSize.y, -epsilon);
		res.max = Vec3(halfSize.x, halfSize.y, epsilon);
		break;

	case NOISE_RECT_ORIENTATION::RECT_XZ:
		res.min = Vec3(-halfSize.x, -epsilon,-halfSize.y);
		res.max = Vec3(halfSize.x, epsilon, halfSize.y);
		break;

	case NOISE_RECT_ORIENTATION::RECT_YZ:
		res.min = Vec3(-epsilon, -halfSize.x, -halfSize.y);
		res.max = Vec3(epsilon, halfSize.x, halfSize.y);
		break;
	}
	return res;
}

N_AABB Noise3D::LogicalRect::ComputeWorldAABB_Accurate()
{
	return ISceneObject::ComputeWorldAABB_Fast();
}

N_BoundingSphere Noise3D::LogicalRect::ComputeWorldBoundingSphere_Accurate()
{
	N_AABB aabb = LogicalRect::ComputeWorldAABB_Accurate();
	N_BoundingSphere sphere;
	sphere.pos = aabb.Centroid();
	sphere.radius = (aabb.max - aabb.Centroid()).Length();
	return sphere;
}


float Noise3D::LogicalRect::ComputeArea()
{
	return mSize.x * mSize.y;
}
