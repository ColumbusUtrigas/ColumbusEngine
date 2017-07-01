#include <Engine.h>

using namespace C;

int main(int argc, char** argv)
{
	C_SDLWindow window(1920, 1080, "Columbus Engine");
	C_EventSystem event;
	event.addWindow(&window);

	C_Shader shader("shader.vert", "shader.frag");

	C_Texture tex("Textures/metal.jpg");
	C_Texture spec("Textures/metal.jpg");

	std::vector<C_Vertex> verts;
	C_Vertex v[36];

	/*v[0].pos = C_Vector3(1, 1, 0);
	v[0].UV = C_Vector2(1, 1);
	v[0].normal = C_Vector3(0, 0, 1);
	v[1].pos = C_Vector3(-1, 1, 0);
	v[1].UV = C_Vector2(0, 1);
	v[1].normal = C_Vector3(0, 0, 1);
	v[2].pos = C_Vector3(-1, -1, 0);
	v[2].UV = C_Vector2(0, 0);
	v[2].normal = C_Vector3(0, 0, 1);

	v[3].pos = C_Vector3(1, 1, 0);
	v[3].UV = C_Vector2(1, 1);
	v[3].normal = C_Vector3(0, 0, 1);
	v[4].pos = C_Vector3(-1, -1, 0);
	v[4].UV = C_Vector2(0, 0);
	v[4].normal = C_Vector3(0, 0, 1);
	v[5].pos = C_Vector3(1, -1, 0);
	v[5].UV = C_Vector2(1, 0);
	v[5].normal = C_Vector3(0, 0, 1);*/

	/*for(int i = 0; i < 6; i++)
		verts.push_back(v[i]);*/

	C_Mesh mesh(Importer::C_LoadOBJVertices("Models/Texture.obj"));
	mesh.mMat.setTexture(&tex);
	mesh.mMat.setSpecMap(&spec);
	mesh.mMat.setColor(C_Vector4(0.3, 0.3, 0.3, 1));
	mesh.mMat.setShader(&shader);

	C_Camera camera;
	C_Render render;
	render.setMainCamera(&camera);
	render.add(&mesh);

	float i = 0;

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
		render.render();

		window.display();

		SDL_Delay(16);
	}

	shader.unbind();

	return 0;
}

