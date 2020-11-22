#include <Graphics/DebugRender.h>
#include <Scene/Transform.h>

namespace Columbus::Graphics
{

	DebugRenderer gDebugRender;

	void DebugRenderer::Initialize()
	{
		RasterizerStateDesc RSD_solid, RSD_wireframe;
		RSD_solid.Cull = CullMode::No;
		RSD_solid.Fill = FillMode::Solid;
		RSD_wireframe.Cull = CullMode::No;
		RSD_wireframe.Fill = FillMode::Wireframe;

		gDevice->CreateRasterizerState(RSD_solid, &RS_solid);
		gDevice->CreateRasterizerState(RSD_wireframe, &RS_wireframe);

		M_cube = gDevice->CreateMesh();
		M_sphere = gDevice->CreateMesh();

		M_cube->Load("Data/Meshes/Box.cmf");
		M_sphere->Load("Data/Meshes/Sphere.cmf");
	}

	void DebugRenderer::NewFrame()
	{
		objects.clear();
	}

	void DebugRenderer::Shutdown()
	{
		delete RS_solid;
		delete RS_wireframe;
		delete M_cube;
		delete M_sphere;
	}

	void DebugRenderer::RenderBox(const Matrix& transform, const Vector4& color, bool wireframe)
	{
		if (_enabled)
		{
			objects.push_back({ transform, color, wireframe ? RS_wireframe : RS_solid, M_cube });
		}
	}

	void DebugRenderer::RenderSphere(const Vector3& pos, float radius, const Vector4& color, bool wireframe)
	{
		if (_enabled)
		{
			Transform transform(pos, {}, { radius });
			objects.push_back({ transform.GetMatrix(), color, wireframe ? RS_wireframe : RS_solid, M_sphere });
		}
	}

}
