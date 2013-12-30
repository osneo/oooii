/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include <oGfx/oGfxManipulator.h>
#include <oGPU/oGPUUtil.h>
#include <oGfx/oGfxPipelines.h>
#include <oGfx/oGfxVertexElements.h>

using namespace ouro;

struct LineListContext
{
	intrusive_ptr<oGPUBuffer> LineList;
	intrusive_ptr<oGPUUtilMesh> CapMesh;
	intrusive_ptr<oGPUUtilMesh> PickMesh;
	oURI URI;
	color LineColor;
};

struct oGfxManipulatorImpl : public oGfxManipulator
{
	enum EVENT
	{
		BEGIN_MANIPULATE,
		ROTATING,
		END_MANIPULATE,
		TRANSLATING,
	};


	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();
	void GetDesc(DESC* _pDesc) const override {Manipulator->GetDesc(_pDesc);}

	bool Pick(uint _ObjectID) override;
	void StopPick() override;

	void SetMousePosition(float2 _position) override;
	void SetViewProjection(float4x4 _ViewMatrixLH, float4x4 _ProjectionMatrixLH) override;
	void GetTransform(float4x4 *_pTransform) const override;
	void SetTransform(const float4x4& _Transform) override;
	
	void GetManipulatorPickLineMeshes(oGPUCommandList* _pCommandList, std::function<void(oGPUUtilMesh* _pMesh, uint _ObjectID)> _Callback) override;
	void GetManipulatorVisualLines(oGPUCommandList* _pCommandList, std::function<void(oGPUBuffer* _pLineList, uint _NumLines)> _Callback) override;
	void GetManipulatorMeshes(oGPUCommandList* _pCommandList, std::function<void(oGPUUtilMesh* _pMesh, float4x4 _Transform, color _MeshColor, uint _ObjectID)> _Callback) override;

	oGfxManipulatorImpl(const char* _Name, const oGfxManipulator::DESC& _Desc, oGPUDevice* _pDevice, bool* _pSuccess);

	template<typename GEOMTRY_T>
	bool CreateGeometryMesh(oGPUDevice* _pDevice, const char* _Name, const GEOMTRY_T& _GeometryDesc, oGPUUtilMesh** _ppMesh);

	template<typename PICK_GEOMTRY_T>
	bool CreateAxisGeometry(oGPUDevice* _pDevice, const char* _BaseName, const PICK_GEOMTRY_T& _PickGeometryDesc, unsigned int _NumLines, oManipulator::AXIS _Axis);
	
	template<typename PICK_GEOMTRY_T, typename CONE_GEOMETRY_T>
	bool CreateAxisGeometry(oGPUDevice* _pDevice, const char* _BaseName, const PICK_GEOMTRY_T& _PickGeometryDesc, const CONE_GEOMETRY_T& _ConeGeometryDesc, unsigned int _NumLines, oManipulator::AXIS _Axis);

	void Update();

	oGfxManipulator::DESC Desc;
	oRefCount RefCount;
	intrusive_ptr<oManipulator> Manipulator;
	intrusive_ptr<oGeometryFactory> GeometryFactory;

	float4x4 Transform;
	float2 CurrentMousePosition;
	float4x4 CurrentViewLH;
	float4x4 CurrentProjectionLH;

	std::vector<float3> VerticesWorking; //just a temp list to use for whatever. member variable to cut down on allocs
	LineListContext Lines[oManipulator::NUM_AXES];
	oManipulator::AXIS PickedAxis;
};


template<typename GEOMTRY_T>
bool oGfxManipulatorImpl::CreateGeometryMesh(oGPUDevice* _pDevice, const char* _Name, const GEOMTRY_T& _GeometryDesc, oGPUUtilMesh** _ppMesh)
{
	oGeometry::LAYOUT GeoLayout;
	GeoLayout.Positions = true;
	GeoLayout.Normals = false;
	GeoLayout.Tangents = false;
	GeoLayout.Texcoords = false;
	GeoLayout.Colors = false;
	GeoLayout.ContinuityIDs = true;

	intrusive_ptr<oGeometry> Geometry; 
	if(!GeometryFactory->Create(_GeometryDesc, GeoLayout, &Geometry))
		return false;

	const oGPU_VERTEX_ELEMENT* pElements = nullptr;
	uint NumElements = 0;
	oGfxGetVertexElements(oGFX_VE_POSITION, &pElements, &NumElements);
	return oGPUUtilMeshCreate(_pDevice, _Name, pElements, NumElements, Geometry, _ppMesh);
}

template<typename PICK_GEOMTRY_T>
bool oGfxManipulatorImpl::CreateAxisGeometry(oGPUDevice* _pDevice, const char* _BaseName, const PICK_GEOMTRY_T& _GeometryDesc, unsigned int _NumLines, oManipulator::AXIS _Axis)
{
	auto& Line = Lines[_Axis];
	
	gpu::buffer_info i;
	i.type = gpu::buffer_type::vertex;
	i.struct_byte_size = sizeof(oGFX_LINE_VERTEX);
	i.array_size = _NumLines * 2;
	
	uri_string Name;
	snprintf(Name, "%s_%s", _BaseName, ouro::as_string(_Axis));
	if(!_pDevice->CreateBuffer(Name, i, &Line.LineList))
		return oErrorPrefixLast("Failed to create a line list for a manipulator: ");

	if(!CreateGeometryMesh(_pDevice, Name, _GeometryDesc, &Line.PickMesh) )
		return oErrorPrefixLast("Failed to create the pick geometry for a manipulator: ");

	Line.URI = Name;
	return true;
}


template<typename PICK_GEOMTRY_T, typename CONE_GEOMETRY_T>
bool oGfxManipulatorImpl::CreateAxisGeometry( oGPUDevice* _pDevice, const char* _BaseName, const PICK_GEOMTRY_T& _PickGeometryDesc, const CONE_GEOMETRY_T& _ConeGeometryDesc, unsigned int _NumLines, oManipulator::AXIS _Axis )
{
	if(!CreateAxisGeometry(_pDevice, _BaseName, _PickGeometryDesc, _NumLines, _Axis))
		return false;

	auto& Line = Lines[_Axis];
	if(!CreateGeometryMesh(_pDevice, _BaseName, _ConeGeometryDesc, &Line.CapMesh) )
		return oErrorPrefixLast("Failed to create the cone for a manipulator: ");

	return true;
}


oGfxManipulatorImpl::oGfxManipulatorImpl(const char* _Name, const oGfxManipulator::DESC& _Desc, oGPUDevice* _pDevice, bool* _pSuccess) 
	: Desc(_Desc)
	, Transform(oIDENTITY4x4)
	, PickedAxis((oManipulator::AXIS)(size_t)ouro::invalid)
{
	if(!oManipulatorCreate(_Desc, &Manipulator))
	{
		oErrorPrefixLast("oGfxManipulator failed to create a manipulator: ");
		return;
	}

	if(!oGeometryFactoryCreate(&GeometryFactory))
	{
		oErrorPrefixLast("oGfxManipulator failed to create a geometry factory: ");
		return;
	}

	switch(Desc.Type)
	{
	case oManipulator::DESC::TRANSLATION:
		{
			oGeometryFactory::RECT_DESC pickd;
			pickd.FaceType = oGeometry::FRONT_CW;
			pickd.Divide = 0;
			pickd.Height = 1;
			pickd.Width = 1;

			oGeometryFactory::CONE_DESC capd;
			capd.FaceType = oGeometry::FRONT_CW;
			capd.Divide = 4;
			capd.Facet = 31;
			capd.Radius = 0.4f;
			capd.Height = 1.2f;
			capd.IncludeBase = false;

			if(!CreateAxisGeometry(_pDevice, _Name, pickd, capd, 1, oManipulator::X))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, capd, 1, oManipulator::Y))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, capd, 1, oManipulator::Z))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, 4, oManipulator::SCREEN))
				return;
		}
		break;
	case oManipulator::DESC::SCALE:
		{
			oGeometryFactory::RECT_DESC pickd;
			pickd.FaceType = oGeometry::FRONT_CW;
			pickd.Divide = 0;
			pickd.Height = 1;
			pickd.Width = 1;

			const float3 BoxSize = float3(0.4f, 0.4f, 0.4f);
			oGeometryFactory::BOX_DESC boxd;
			boxd.FaceType = oGeometry::FRONT_CW;
			boxd.Divide = 4;
			boxd.Bounds = oAABoxf(oAABoxf::min_max, -BoxSize, BoxSize);

			if(!CreateAxisGeometry(_pDevice, _Name, pickd, boxd, 1, oManipulator::X))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, boxd, 1, oManipulator::Y))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, boxd, 1, oManipulator::Z))
				return;

			// Special case for screen
			{
				boxd.Bounds = oAABoxf(oAABoxf::min_max, -5.0f * BoxSize, 5.0f * BoxSize);
				if(!CreateGeometryMesh(_pDevice, _Name, boxd, &Lines[oManipulator::SCREEN].CapMesh) )
					return;

				uri_string AxisName;
				snprintf(AxisName, "%s_%s", _Name, ouro::as_string(oManipulator::SCREEN));
				Lines[oManipulator::SCREEN].URI = AxisName;
			}

		}
		break;
	case oManipulator::DESC::ROTATION:
		{
			oGeometryFactory::TORUS_DESC pickd;
			pickd.FaceType = oGeometry::FRONT_CW;
			pickd.InnerRadius = 5;
			pickd.OuterRadius = 7;
			pickd.Divide = oManipulator::ROTATION_PICK_TORUS_DIVIDE;
			pickd.Facet = oManipulator::ROTATION_PICK_TORUS_FACET;
			pickd.Color = White;

			if(!CreateAxisGeometry(_pDevice, _Name, pickd, oManipulator::ROTATION_CIRCLE_VCOUNT*2, oManipulator::X))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, oManipulator::ROTATION_CIRCLE_VCOUNT*2, oManipulator::Y))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, oManipulator::ROTATION_CIRCLE_VCOUNT*2, oManipulator::Z))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, oManipulator::ROTATION_CIRCLE_VCOUNT*2, oManipulator::SCREEN))
				return;

		}
		break;
		oNODEFAULT;
	}

	// This ensures nothing is picked
	Pick(0); 
	*_pSuccess = true;
}

bool oGfxManipulatorImpl::Pick(uint _ObjectID)
{
	static const color unpickedColors[oManipulator::NUM_AXES] = {Red, Green, Blue, Gray};
	for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
		Lines[i].LineColor = unpickedColors[i];

	if(0 != _ObjectID)
	{
		for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
		{
			oManipulator::AXIS Axis = (oManipulator::AXIS)i;
			auto& Line = Lines[Axis];
			if(_ObjectID == (uint)Line.URI.Hash())
			{
				Line.LineColor = Yellow;
				if(PickedAxis != Axis)
				{
					PickedAxis = Axis;
					Manipulator->BeginManipulation(Axis, CurrentMousePosition);
				}
				return true;
			}
		}
	}
	return false;
}

void oGfxManipulatorImpl::StopPick()
{
	Manipulator->GetTransform(&Transform);
	Manipulator->EndManipulation();
	PickedAxis = (oManipulator::AXIS)(size_t)ouro::invalid;
	Update();
}

void oGfxManipulatorImpl::SetMousePosition(float2 _position)
{
	CurrentMousePosition = _position;
	Update();
}

void oGfxManipulatorImpl::SetTransform(const float4x4& _Transform)
{
	Transform = _Transform;
	Update();
}

void oGfxManipulatorImpl::GetTransform(float4x4 *_pTransform) const
{ 
	Manipulator->GetTransform(_pTransform);
}

void oGfxManipulatorImpl::SetViewProjection(float4x4 _ViewMatrixLH, float4x4 _ProjectionMatrixLH)
{
	CurrentViewLH = _ViewMatrixLH;
	CurrentProjectionLH = _ProjectionMatrixLH;
	Update();
}

void oGfxManipulatorImpl::Update()
{
	Manipulator->Update(CurrentMousePosition, Transform, CurrentViewLH, CurrentProjectionLH);
}

void oGfxManipulatorImpl::GetManipulatorPickLineMeshes(oGPUCommandList* _pCommandList, std::function<void(oGPUUtilMesh* _pMesh, uint _ObjectID)> _Callback)
{
	size_t MaxNumLines, MaxNumPickVerts;
	Manipulator->GetMaxSizes(MaxNumLines, MaxNumPickVerts);
	VerticesWorking.resize(MaxNumPickVerts);

	for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
	{
		oManipulator::AXIS Axis = (oManipulator::AXIS)i;
		auto& Line = Lines[Axis];
		auto& Mesh = Line.PickMesh;
		if(Mesh && !Manipulator->IsClipped(Axis))
		{
			size_t PickVertexCount;
			Manipulator->GetLinesPickGeometry(Axis, &VerticesWorking[0], PickVertexCount);
			if(PickVertexCount > 0)
			{
				oGPUUtilMesh::DESC MeshDesc;
				Mesh->GetDesc(&MeshDesc);

				ouro::surface::const_mapped_subresource msr;
				msr.data = &VerticesWorking[0];
				msr.row_pitch = sizeof(VerticesWorking[0]);
				msr.depth_pitch = MeshDesc.NumVertices * msr.row_pitch;
				_pCommandList->Commit(Mesh->GetVertexBuffer(), 0, msr);
				_Callback(Mesh, (uint)Line.URI.Hash());
			}
		}
	}

}

void oGfxManipulatorImpl::GetManipulatorVisualLines(oGPUCommandList* _pCommandList, std::function<void(oGPUBuffer* _pLineList, uint _NumLines)> _Callback)
{
	size_t MaxNumLines, MaxNumPickVerts;
	Manipulator->GetMaxSizes(MaxNumLines, MaxNumPickVerts);
	VerticesWorking.resize(MaxNumLines);

	for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
	{
		oManipulator::AXIS Axis = (oManipulator::AXIS)i;
		auto& Line = Lines[Axis];
		if(Line.LineList && !Manipulator->IsClipped(Axis))
		{
			size_t LineVertexCount;
			Manipulator->GetLines(Axis, &VerticesWorking[0], LineVertexCount);
			if(LineVertexCount > 0)
			{
				ouro::surface::mapped_subresource msr;
				_pCommandList->Reserve(Line.LineList, 0, &msr);

				auto LineCount = LineVertexCount == 2 ? 1 : LineVertexCount;

				for(size_t i = 0; i < LineCount; ++i)
				{
					((oGFX_LINE_VERTEX*)msr.data)[2 * i].LSPosition = VerticesWorking[i];
					((oGFX_LINE_VERTEX*)msr.data)[2 * i].Color = Line.LineColor;

					((oGFX_LINE_VERTEX*)msr.data)[2 * i + 1].LSPosition = VerticesWorking[(i + 1)%LineVertexCount];
					((oGFX_LINE_VERTEX*)msr.data)[2 * i + 1].Color = Line.LineColor;
				}

				_pCommandList->Commit(Line.LineList, 0, msr, ouro::surface::box(as_int(LineCount * 2)));
				_Callback(Line.LineList, as_uint(LineCount));
			}
		}
	}
}

void oGfxManipulatorImpl::GetManipulatorMeshes(oGPUCommandList* _pCommandList, std::function<void(oGPUUtilMesh* _pMesh, float4x4 _Transform, color _MeshColor, uint _ObjectID)> _Callback)
{
	for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
	{
		oManipulator::AXIS Axis = (oManipulator::AXIS)i;
		auto& Line = Lines[Axis];
		auto& Cap = Line.CapMesh;
		if(Cap)
		{
			float4x4 CapTransform;
			Manipulator->GetCapTransform(Axis, CapTransform);
			_Callback(Cap, CapTransform, Line.LineColor, (uint)Line.URI.Hash());
		}
	}
}

bool oGfxManipulatorCreate(const char* _Name, const oGfxManipulator::DESC& _Desc, oGPUDevice* _pDevice, oGfxManipulator** _ppManipulator)
{
	bool success = false;
	oCONSTRUCT(_ppManipulator, oGfxManipulatorImpl(_Name, _Desc, _pDevice, &success));
	return success;
}
