#include <Engine.h>

using namespace C;

int main(int argc, char** argv)
{
	C_SDLWindowConfig config;;
	config.Resizable = true;

	C_SDLWindow window(config);
	C_EventSystem event;
	event.addWindow(&window);

	C_Shader shader("Data/Shaders/shader.vert", "Data/Shaders/shader.frag");

	C_Texture tex("Data/Textures/metal.jpg");
	C_Texture spec("Data/Textures/metal.jpg");

	C_Mesh mesh(Importer::C_LoadOBJVertices("Data/Models/Texture.obj"));
	mesh.mMat.setTexture(&tex);
	mesh.mMat.setSpecMap(&spec);
	mesh.mMat.setColor(C_Vector4(0.3, 0.3, 0.3, 1));
	mesh.mMat.setShader(&shader);
	mesh.setPos(C_Vector3(2, 0, 0));

	C_Camera camera;
	C_Render render;
	render.setMainCamera(&camera);
	render.add(&mesh);

	float i = 0;

	C_CubemapPath cpath = 
	{
		"Data/Skyboxes/4/r.tga",
		"Data/Skyboxes/4/l.tga",
		"Data/Skyboxes/4/u.tga",
		"Data/Skyboxes/4/d.tga",
		"Data/Skyboxes/4/b.tga",
		"Data/Skyboxes/4/f.tga",
	};


	C_Cubemap cubemap(cpath);
	C_Skybox skybox(&cubemap);

	mesh.mMat.setReflection(&cubemap);

	render.setSkybox(&skybox);
	
	C_ParticleEmitter particles(1000);

	while (window.isOpen())
	{
		event.pollEvents();

		window.clear(0, 0, 0.75, 1);

		C_SetPerspective(60, window.aspect(), 0.00001, 100000);

		if (window.getKey(SDL_SCANCODE_W))
			camera.addPos(camera.direction() * 0.1);
		if (window.getKey(SDL_SCANCODE_S))
			camera.addPos(C_Vector3(0, 0, 0) - camera.direction() * 0.1);
		if (window.getKey(SDL_SCANCODE_A))
			camera.addPos(C_Vector3(0, 0, 0) - camera.right() * 0.1);
		if (window.getKey(SDL_SCANCODE_D))
			camera.addPos(camera.right() * 0.1);
		if (window.getKey(SDL_SCANCODE_UP))
			camera.addRot(C_Vector3(-2.5, 0, 0));
		if (window.getKey(SDL_SCANCODE_DOWN))
			camera.addRot(C_Vector3(2.5, 0, 0));
		if (window.getKey(SDL_SCANCODE_LEFT))
			camera.addRot(C_Vector3(0, 2.5, 0));
		if (window.getKey(SDL_SCANCODE_RIGHT))
			camera.addRot(C_Vector3(0, -2.5, 0));

		if (window.getKey(SDL_SCANCODE_LSHIFT))
			camera.addPos(C_Vector3(0, 0, 0) - camera.up() * 0.1);
		if (window.getKey(SDL_SCANCODE_SPACE))
			camera.addPos(camera.up() * 0.1);
		if (window.getKey(SDL_SCANCODE_Q))
			camera.addRot(C_Vector3(0, 0, 2.5));
		if (window.getKey(SDL_SCANCODE_E))
			camera.addRot(C_Vector3(0, 0, -2.5));
		
		camera.update();

		if(window.getKeyDown(SDL_SCANCODE_V))
			printf("Key down\n");
		if (window.getKeyUp(SDL_SCANCODE_V))
			printf("Key up\n");

		//mesh.addRot(C_Vector3(1, 1, 0));

		//mesh.draw();
		skybox.draw();
		
		particles.draw();

		render.render();

		window.display();

		SDL_Delay(16);
	}

	shader.unbind();

	return 0;
}

