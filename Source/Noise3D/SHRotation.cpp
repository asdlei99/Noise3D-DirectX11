
/***********************************************************************

			Spherical Harmonic Rotation implementation

************************************************************************/

#include "Noise3D.h"

using namespace Noise3D;

Noise3D::GI::SHRotationWignerMatrix::SHRotationWignerMatrix(uint32_t highestBandIndex)
{
	mHighestBandIndex = highestBandIndex;
	if (highestBandIndex >= 100)
	{
		ERROR_MSG("Wigner Matrix(SH Rotation): band width is too large. Try less bands.");
		mMat.resize(0);
	}
	else
	{
		mMat.resize(mHighestBandIndex+1);
		for (uint32_t i = 0; i <= highestBandIndex; ++i)
		{
			mMat.at(i).resize((2 * i + 1)*(2 * i + 1), 0.0f);
		}
	}
}

float Noise3D::GI::SHRotationWignerMatrix::GetByRowCol(uint32_t l, uint32_t row, uint32_t col)
{
	//the l-th (l>=0) band Wigner Matrix
	return mMat.at(l).at((2*l +1) * row + col);
}

float Noise3D::GI::SHRotationWignerMatrix::GetByIndex(uint32_t l, int m, int n)
{
	//in l-th band Wigner Matrix, -L<=m<=L 
	return mMat.at(l).at((2 * l + 1)*(int(l) - m) + (int(l) - n)) ;
}

void Noise3D::GI::SHRotationWignerMatrix::SetByRowCol(uint32_t l, uint32_t row, uint32_t col,float val)
{
	//the l-th (l>=0) band wigner matrix
	mMat.at(l).at((2 * l + 1) * row + col) = val;
}

void Noise3D::GI::SHRotationWignerMatrix::SetByIndex(uint32_t l, int m, int n, float val)
{
	//the l-th (l>=0) band wigner matrix
	mMat.at(l).at((2 * l + 1)*(int(l) - m) + (int(l) - n)) = val;
}

void Noise3D::GI::SHRotationWignerMatrix::Rotate(RigidTransform t, std::vector<NColor4f>& in_out_SHVector)
{
	//0. ZYZ euler angle decomposition
	N_EULER_ANGLE_ZYZ euler = t.GetEulerAngleZYZ();

	//1. Rotate Z (without matrix construction)
	mFunction_RotateZ(euler.angleZ1, in_out_SHVector);

	//2. Rotate Y (explicitly construct Wigner Matrix)
	//reference : Lisle's <Algorithms for Spherical Harmonic Lighting>
	mFunction_RotateY(euler.angleY2, in_out_SHVector);

	//3. Rotate Z (without matrix construction)
	mFunction_RotateZ(euler.angleZ3, in_out_SHVector);

}

/**********************************************

							PRIVATE

***********************************************/
void Noise3D::GI::SHRotationWignerMatrix::mFunction_RotateZ(float angle, std::vector<NColor4f>& in_out_SHVector)
{
	//WARNING: SHVector starts from -m to m (different convention from Wigner Matrix)
	//Reference: <Fast Approximation to Spherical Harmonic Rotation> Krivanek, Appendix B
	for (uint32_t L= 0; L <= mHighestBandIndex; ++L)
	{
		for (int M = 1; M <=  int(L); ++M)
		{
			NColor4f c_L_negM = in_out_SHVector.at(GI::SH_FlattenIndex(L, -M));//element of index -m in this band
			NColor4f c_L_posM = in_out_SHVector.at(GI::SH_FlattenIndex(L, M));//element of index m in this band
			float cos_m_alpha = cos(M * angle);
			float sin_m_alpha = sin(M * angle);
			in_out_SHVector.at(GI::SH_FlattenIndex(L, M)) = c_L_negM * cos_m_alpha - c_L_posM *sin_m_alpha;
			in_out_SHVector.at(GI::SH_FlattenIndex(L, M)) = c_L_negM * sin_m_alpha + c_L_posM * cos_m_alpha;
		}
	}
}

void Noise3D::GI::SHRotationWignerMatrix::mFunction_RotateY(float angle, std::vector<NColor4f>& in_out_SHVector)
{

	//1. constructing Wigner Matrix - Rotation Y
	if (mHighestBandIndex >= 0)
	{
		SHRotationWignerMatrix::SetByRowCol(0, 0, 0, 1.0f);
	}
	
	if (mHighestBandIndex >= 1)
	{
		//(2018.10.27)band 1 wigner matrix of rotation Y remains un-determined. Can't find support literature.
		//reference: Lisle's <Algorithms for Spherical Harmonic Lighting>�� but don't know the correspondence between RowCol & index
		SHRotationWignerMatrix::SetByRowCol(1, 0, 0, 1.0f);
		SHRotationWignerMatrix::SetByRowCol(1, 0, 1, 0.0f);
		SHRotationWignerMatrix::SetByRowCol(1, 0, 2, 0.0f);

		SHRotationWignerMatrix::SetByRowCol(1, 1, 0, 0.0f);
		SHRotationWignerMatrix::SetByRowCol(1, 1, 1, cos(angle));
		SHRotationWignerMatrix::SetByRowCol(1, 1, 2, sin(angle));

		SHRotationWignerMatrix::SetByRowCol(1, 2, 0, 0.0f);
		SHRotationWignerMatrix::SetByRowCol(1, 2, 1, -sin(angle));
		SHRotationWignerMatrix::SetByRowCol(1, 2, 2, cos(angle));

		/*
		SHRotationWignerMatrix::SetByRowCol(1, 2, 2, 1.0f);
		SHRotationWignerMatrix::SetByRowCol(1, 2, 1, 0.0f);
		SHRotationWignerMatrix::SetByRowCol(1, 2, 0, 0.0f);

		SHRotationWignerMatrix::SetByRowCol(1, 1, 2, 0.0f);
		SHRotationWignerMatrix::SetByRowCol(1, 1, 1, cos(angle));
		SHRotationWignerMatrix::SetByRowCol(1, 1, 0, sin(angle));

		SHRotationWignerMatrix::SetByRowCol(1, 0, 1, 0.0f);
		SHRotationWignerMatrix::SetByRowCol(1, 0, 1, -sin(angle));
		SHRotationWignerMatrix::SetByRowCol(1, 0, 0, cos(angle));
		*/
	}

	if(mHighestBandIndex >= 2)
	{

	}

	//2. Multiplication
	for (int l = 0; l <= mHighestBandIndex; ++l)
	{
		//for every SH vector band, assign the result of WignerMatrix * band (manipulate 4 channels simultaneously)
		for (int m = 0; m <= mHighestBandIndex; ++m)
		{
			NColor4f result = NColor4f(0,0,0,0);
			for (int n = -l; n <= l; ++n)
			{
				result += in_out_SHVector.at(GI::SH_FlattenIndex(l, n)) * SHRotationWignerMatrix::GetByIndex(l, m, n);
			}
			in_out_SHVector.at(GI::SH_FlattenIndex(l, m)) = result;
		}
	}
}
