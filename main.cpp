#include <Engine.h>

using namespace C;

int FPS_LIMIT = 120;

int main(int argc, char** argv)
{
	C_SDLWindowConfig config;;
	config.Resizable = true;

	C_SDLWindow window(config);
	C_EventSystem event;
	event.addWindow(&window);

	C_Shader shader("Data/Shaders/standart.vert", "Data/Shaders/standart.frag");

	C_TextureManager textureManager;

	C_Texture tex("Data/Textures/metal2.jpg");
	C_Texture spec("Data/Textures/metal2-spec.jpg");
	C_Texture norm("Data/Textures/metal2-normal.jpg");

	C_Texture tex2("Data/Textures/milos.jpg");
	C_Texture spec2("Data/Textures/milos-spec.jpg");
	C_Texture norm2("Data/Textures/milos-normal.jpg");

	textureManager.add(&tex);
	textureManager.add(&spec);
	textureManager.add(&norm);

	textureManager.add(&tex2);
	textureManager.add(&spec2);
	textureManager.add(&norm2);

	Importer::C_Importer importer;
	importer.load("Data/Models/ASD.obj");


	C_Mesh mesh(importer.mVertices[0]);
	C_Mesh mesh2(importer.mVertices[1]);
	C_Mesh mesh3(C_PrimitiveBox(C_Vector3(5, 1, 5)));

	mesh.mMat.loadFromXML("Data/Materials/Default.mtl");
	mesh2.mMat.loadFromXML("Data/Materials/Default.mtl");
	mesh3.mMat.loadFromXML("Data/Materials/Default.mtl");

	mesh.mMat.setTexture(&tex);
	mesh.mMat.setSpecMap(&spec);
	mesh.mMat.setNormMap(&norm);
	mesh.mMat.setShader(&shader);
	mesh.setPos(C_Vector3(0, 0, 0));

	mesh2.mMat.setTexture(&tex2);
	mesh2.mMat.setSpecMap(&spec2);
	mesh2.mMat.setNormMap(&norm2);
	mesh2.mMat.setShader(&shader);

	mesh3.mMat.setColor(C_Vector4(1, 1, 1, 1));
	mesh3.mMat.setTexture(&tex2);
	mesh3.mMat.setSpecMap(&spec2);
	mesh3.mMat.setNormMap(&norm2);
	mesh3.mMat.setShader(&shader);
	mesh3.setPos(C_Vector3(0, -2, 0));

	mesh.addChild(&mesh2);


	C_Camera camera;
	C_Render render;
	render.setMainCamera(&camera);
	render.add(&mesh);
	render.add(&mesh2);
	render.add(&mesh3);

	float i = 0;

	C_CubemapPath cpath =
	{
		"Data/Skyboxes/5/r.jpg",
		"Data/Skyboxes/5/l.jpg",
		"Data/Skyboxes/5/d.jpg",
		"Data/Skyboxes/5/u.jpg",
		"Data/Skyboxes/5/b.jpg",
		"Data/Skyboxes/5/f.jpg",
	};


	C_Cubemap cubemap(cpath);
	C_Skybox skybox(&cubemap);

	mesh.mMat.setReflection(&cubemap);
	//mesh2.mMat.setReflection(&cubemap);
	//mesh3.mMat.setReflection(&cubemap);

	render.setSkybox(&skybox);

	C_Material partmat;
	C_Texture partex("Data/Textures/smoke.png");
	textureManager.add(&partex);
	partmat.setTexture(&partex);
	partmat.setColor(C_Vector4(1.0, 1.0, 1.0, 0.5));


	C_ParticleEffect particleEffect;
	particleEffect.loadFromXML("Data/Particles/A.particles");
	particleEffect.setMaterial(&partmat);
	C_ParticleEmitter particles(&particleEffect);

	//particleEffect.saveToXLM("Data/Particles/A.particles");

	C_Timer timer;

	int FPS = 0;

	//window.setVerticalSync(true);
	window.setFPSLimit(60);

	render.add(&particles);

	while (window.isOpen())
	{
		float RedrawTime = window.getRedrawTime();

		event.pollEvents();

		window.clear(0, 0, 0.75, 1);

		C_SetPerspective(60, window.aspect(), 0.001, 1000);

		if (window.getKey(SDL_SCANCODE_W))
			camera.addPos(camera.direction() * RedrawTime * 5);
		if (window.getKey(SDL_SCANCODE_S))
			camera.addPos(C_Vector3(0, 0, 0) - camera.direction() * RedrawTime * 5);
		if (window.getKey(SDL_SCANCODE_A))
			camera.addPos(C_Vector3(0, 0, 0) - camera.right() * RedrawTime * 5);
		if (window.getKey(SDL_SCANCODE_D))
			camera.addPos(camera.right() * RedrawTime * 5);
		if (window.getKey(SDL_SCANCODE_UP))
			camera.addRot(C_Vector3(-125 * RedrawTime, 0, 0));
		if (window.getKey(SDL_SCANCODE_DOWN))
			camera.addRot(C_Vector3(125 * RedrawTime, 0, 0));
		if (window.getKey(SDL_SCANCODE_LEFT))
			camera.addRot(C_Vector3(0, 125 * RedrawTime, 0));
		if (window.getKey(SDL_SCANCODE_RIGHT))
			camera.addRot(C_Vector3(0, -125 * RedrawTime, 0));

		if (window.getKey(SDL_SCANCODE_I))
			particleEffect.addPos(C_Vector3(0, 0, -3) * RedrawTime);
		if (window.getKey(SDL_SCANCODE_K))
			particleEffect.addPos(C_Vector3(0, 0, 3) * RedrawTime);
		if (window.getKey(SDL_SCANCODE_J))
			particleEffect.addPos(C_Vector3(-3, 0, 0) * RedrawTime);
		if (window.getKey(SDL_SCANCODE_L))
			particleEffect.addPos(C_Vector3(3, 0, 0) * RedrawTime);

		if (window.getKey(SDL_SCANCODE_LSHIFT))
			camera.addPos(C_Vector3(0, 0, 0) - camera.up() * RedrawTime * 5);
		if (window.getKey(SDL_SCANCODE_SPACE))
			camera.addPos(camera.up() * RedrawTime * 5);
		if (window.getKey(SDL_SCANCODE_Q))
			camera.addRot(C_Vector3(0, 0, 125 * RedrawTime));
		if (window.getKey(SDL_SCANCODE_E))
			camera.addRot(C_Vector3(0, 0, -125 * RedrawTime));

		camera.update();

		if(window.getKeyDown(SDL_SCANCODE_V))
			printf("Key down\n");
		if (window.getKeyUp(SDL_SCANCODE_V))
			printf("Key up\n");

		//mesh2.addRot(C_Vector3(0, 1, 0));

		render.setWindowSize(window.getSize());
		render.render();

		//particles.draw();

		window.display();

		if ((timer.elapsed()) > 1.0)
		{
			printf("%i\n", window.getFPS());
			timer.reset();
		}
	}

	shader.unbind();

	return 0;
}
