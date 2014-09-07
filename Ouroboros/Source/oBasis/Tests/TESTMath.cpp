// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/macros.h>
#include <oCompute/oComputeUtil.h>
#include <oBasis/oMath.h>
#include <oBasis/oError.h>
#include "oBasisTestCommon.h"

using namespace ouro;

#define oTESTB2(test) do { if (!(test)) return false; } while(false)

bool Test_decompose()
{
	float3 testRotDegrees = float3(45.0f, 32.0f, 90.0f);
	float3 testTx = float3(-14.0f, 70.0f, 32.0f);
	float3 testScale = float3(17.0f, 2.0f, 3.0f); // negative scale not supported

	// Remember assembly of the matrix must be in the canonical order for the 
	// same values to come back out through decomposition.
	float4x4 m = make_scale(testScale) * make_rotation(radians(testRotDegrees)) * make_translation(testTx);

	float shearXY, shearXZ, shearZY;
	float3 scale, rot, tx;
	float4 persp;
	decompose(m, &scale, &shearXY, &shearXZ, &shearZY, &rot, &tx, &persp);

	rot = degrees(rot);
	oTESTB(equal(testScale, scale), "Scales are not the same through assembly and decomposition");
	oTESTB(equal(testRotDegrees, rot), "Rotations are not the same through assembly and decomposition");
	oTESTB(equal(testTx, tx), "Translations are not the same through assembly and decomposition");

	oErrorSetLast(0, "");
	return true;
}

bool Test_SplitRectOverlap()
{
	static const int NumberOfRects = 4;
	ouro::rect BrokenRects[NumberOfRects];
	float BrokenRectWeights[NumberOfRects];
	float fEvenWeight = 1.0f / (float)NumberOfRects;

	for(int i = 0; i < NumberOfRects; ++i)
		BrokenRectWeights[i] = fEvenWeight;

	for(int i = 0; i < NumberOfRects; ++i)
	{
		float fRand = (float)rand() / (float)RAND_MAX;
		int iRand = (int)(fRand * (NumberOfRects - 1));
		float& StolenValue =  BrokenRectWeights[iRand];
		float StolenWeight = StolenValue * fRand;
		StolenValue -= StolenWeight;
		BrokenRectWeights[i] += StolenWeight;
		// Randomly steal weight

	}
	float totalWeight = 0.0f;
	for(int i = 0; i < NumberOfRects; ++i)
		totalWeight += BrokenRectWeights[i];

	oTESTB(equal(totalWeight, 1.0f, 20), "Test failed to distribute random weight evenly (totalWeight = %f)", totalWeight);

	ouro::rect srcRect;
	srcRect.Min = 0;
	srcRect.Max = int2(320, 480);
	oTESTB(SplitRect(srcRect, NumberOfRects, BrokenRectWeights, 1, 1, BrokenRects), "Failed to split rectangles in an efficient way");

	oErrorSetLast(0, "");
	return true;
}

bool Test_Frustum()
{
	static const planef EXPECTED_PLANES_LH[6] =
	{
		planef(1.0f, 0.0f, 0.0f,  1.0f),
		planef(-1.0f, 0.0f, 0.0f, 1.0f),
		planef(0.0f, -1.0f, 0.0f, 1.0f),
		planef(0.0f, 1.0f, 0.0f,  1.0f),
		planef(0.0f, 0.0f, 1.0f,  1.0f),
		planef(0.0f, 0.0f, -1.0f,  1.0f),
	};

	static const planef EXPECTED_PLANES_RH[6] =
	{
		planef(1.0f, 0.0f, 0.0f,  1.0f),
		planef(-1.0f, 0.0f, 0.0f, 1.0f),
		planef(0.0f, -1.0f, 0.0f, 1.0f),
		planef(0.0f, 1.0f, 0.0f,  1.0f),
		planef(0.0f, 0.0f, -1.0f, 1.0f),
		planef(0.0f, 0.0f, 1.0f,  1.0f),
	};

	static const planef EXPECTED_PERSPECTIVE_PLANES_LH[6] =
	{
		planef(0.707f, 0.0f, 0.707f, 0.0f),
		planef(-0.707f, 0.0f, 0.707f, 0.0f),
		planef(0.0f, -0.707f, 0.707f, 0.0f),
		planef(0.0f, 0.707f, 0.707f, 0.0f),
		planef(0.0f, 0.0f, 1.0f,    -0.1f),
		planef(0.0f, 0.0f, -1.0f,  100.0f),
	};

	static const planef EXPECTED_PERSPECTIVE_PLANES_RH[6] =
	{
		planef(0.707f, 0.0f, -0.707f, 0.0f),
		planef(-0.707f, 0.0f, -0.707f, 0.0f),
		planef(0.0f, -0.707f, -0.707f, 0.0f),
		planef(0.0f, 0.707f, -0.707f, 0.0f),
		planef(0.0f, 0.0f, -1.0f, -0.1f),
		planef(0.0f, 0.0f, 1.0f, 100.0f),
	};

	static const char* names[6] =
	{
		"left",
		"right",
		"top",
		"bottom",
		"near",
		"far",
	};
                                                   
	const int MAX_ULPS = 1800;

	planef planes[6];
	float4x4 P = make_orthographic_lh(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	oExtractFrustumPlanes(planes, P, false);
	for (auto i = 0; i < 6; i++)
		oTESTB(equal(EXPECTED_PLANES_LH[i], planes[i], MAX_ULPS), "orthographic LH %s plane comparison failed", names[i]);

	P = make_orthographic_rh(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	oExtractFrustumPlanes(planes, P, false);
	for (auto i = 0; i < 6; i++)
		oTESTB(equal(EXPECTED_PLANES_RH[i], planes[i], MAX_ULPS), "orthographic RH %s plane comparison failed", names[i]);

	P = make_perspective_lh(radians(90.0f), 1.0f, 0.1f, 100.0f);
	oExtractFrustumPlanes(planes, P, true);
	for (auto i = 0; i < 6; i++)
		oTESTB(equal(EXPECTED_PERSPECTIVE_PLANES_LH[i], planes[i], MAX_ULPS), "perspective LH %s plane comparison failed", names[i]);

	P = make_perspective_rh(radians(90.0f), 1.0f, 0.1f, 100.0f);
	oExtractFrustumPlanes(planes, P, true);
	for (auto i = 0; i < 6; i++)
		oTESTB(equal(EXPECTED_PERSPECTIVE_PLANES_RH[i], planes[i], MAX_ULPS), "perspective RH %s plane comparison failed", names[i]);

	oErrorSetLast(0, "");
	return true;
}

namespace ouro { const char* as_string(const oFRUSTUM_CORNER& _Corner); } // @tony: why is this necessary?

bool Test_FrustumCalcCorners()
{
	static const float3 EXPECTED_ORTHO_LH[8] =
	{
		float3(-1.0f, 1.0f, -1.0f),
		float3(-1.0f, 1.0f, 1.0f),
		float3(-1.0f, -1.0f, -1.0f),
		float3(-1.0f, -1.0f, 1.0f),
		float3(1.0f, 1.0f, -1.0f),
		float3(1.0f, 1.0f, 1.0f),
		float3(1.0f, -1.0f, -1.0f),
		float3(1.0f, -1.0f, 1.0f),
	};

	static const float3 EXPECTED_VP_PROJ_LH[8] =
	{
		float3(9.9f, 0.1f, -10.1f),
		float3(-90.0f, 100.0f, -110.0f),
		float3(9.9f, -0.1f, -10.1f),
		float3(-90.0f, -100.0f, -110.0f),
		float3(9.9f, 0.1f, -9.9f),
		float3(-90.0f, 100.0f, 90.0f),
		float3(9.9f, -0.1f, -9.9f),
		float3(-90.0f, -100.0f, 90.0f),
	};

	const int MAX_ULPS = 1800;

	float4x4 P = make_orthographic_lh(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	oTESTB(!oHasPerspective(P), "Ortho matrix says it has perspective when it doesn't");
	oFrustumf f(P);
	float3 corners[8];
	oTESTB(oExtractFrustumCorners(f, corners), "oExtractFrustumCorners failed (f)");
	for (auto i = 0; i < 8; i++)
		oTESTB(equal(EXPECTED_ORTHO_LH[i], corners[i], MAX_ULPS), "corner %s mismatch", as_string((oFRUSTUM_CORNER)i));

	P = make_perspective_lh(radians(90.0f), 1.0f, 0.1f, 100.0f);
	float4x4 V = make_lookat_lh(float3(10.0f, 0.0f, -10.0f), float3(0.0f, 0.0f, -10.0f), float3(0.0f, 1.0f, 0.0f));
	float4x4 VP = V * P;
	oFrustumf wsf(VP);
	oTESTB(oExtractFrustumCorners(wsf, corners), "oExtractFrustumCorners failed (wsf)");

	float nearWidth = distance(corners[oFRUSTUM_RIGHT_TOP_NEAR], corners[oFRUSTUM_LEFT_TOP_NEAR]);
	float farWidth = distance(corners[oFRUSTUM_RIGHT_TOP_FAR], corners[oFRUSTUM_LEFT_TOP_FAR]);
	oTESTB(equal(0.2f, nearWidth, MAX_ULPS), "width of near plane incorrect");
	oTESTB(equal(200.0f, farWidth, MAX_ULPS), "width of far plane incorrect");
	for (auto i = 0; i < 8; i++)
		oTESTB(equal(EXPECTED_VP_PROJ_LH[i], corners[i], MAX_ULPS), "corner %s mismatch", as_string((oFRUSTUM_CORNER)i));

	// Verify world pos extraction math.  This is used in deferred rendering.
	const float3 TestPos = float3(-30, 25, 41);
	float3 ComputePos;
	{
		oFrustumf psf(P);
		float3 EyePos = extract_eye(V);
		float EyeZ = mul(V, float4(TestPos, 1.0f)).z;
		float InverseFarPlaneDistance = 1.0f / distance(wsf.Far, EyePos);
		float Depth = EyeZ * InverseFarPlaneDistance;

		float4 ProjTest = mul(VP, float4(TestPos, 1.0f));
		ProjTest /= ProjTest.w;

		float2 LerpFact = ProjTest.xy() * 0.5f + 0.5f;
		LerpFact.y = 1.0f - LerpFact.y;

		oFrustumf Frust(P);
		oTESTB(oExtractFrustumCorners(Frust, corners), "oExtractFrustumCorners failed (Frust)");
		float3x3 FrustTrans = (float3x3)invert(V);
		float3 LBF = mul(FrustTrans, corners[oFRUSTUM_LEFT_BOTTOM_FAR]);
		float3 RBF = mul(FrustTrans, corners[oFRUSTUM_RIGHT_BOTTOM_FAR]);
		float3 LTF = mul(FrustTrans, corners[oFRUSTUM_LEFT_TOP_FAR]);
		float3 RTF = mul(FrustTrans, corners[oFRUSTUM_RIGHT_TOP_FAR]);

		float3 FarPointTop = lerp(LTF,  RTF, LerpFact.x);
		float3 FarPointBottom = lerp(LBF, RBF, LerpFact.x);
		float3 FarPoint = lerp(FarPointTop, FarPointBottom, LerpFact.y);

		ComputePos = EyePos + (FarPoint * Depth);
	}
	oTESTB(equal(ComputePos, TestPos, MAX_ULPS), "Computed world pos is incorrect");

	oErrorSetLast(0, "");
	return true;
}

bool Test_MatrixMath()
{
	const int MAX_ULPS = 1800;
	float3 TestVec = normalize(float3(-54, 884, 32145));
	float3 TargetVecs[] =
	{
		TestVec, // Identity
		-TestVec, // Flip
		normalize(float3(TestVec.y, TestVec.z, TestVec.x)), // Axis swap
		normalize(float3(-242, 1234, 3)) // Other
	};

	int i = 0;
	for (const auto& t : TargetVecs)
	{
		float4x4 Rotation = make_rotation(TestVec, t);
		float3 RotatedVec = mul(Rotation, TestVec);
		oTESTB(equal(RotatedVec, t, MAX_ULPS), "make_rotation failed with #%d (%f %f %f)", i, t.x, t.y, t.z);
		i++;
	}
	
	return true;
}


bool oBasisTest_oMath()
{
	oTESTB2(Test_MatrixMath());
	oTESTB2(Test_decompose());
	oTESTB2(Test_SplitRectOverlap());
	oTESTB2(Test_Frustum());
	oTESTB2(Test_FrustumCalcCorners());
	return true;
}
