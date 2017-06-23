#include <Engine.h>

using namespace C;

int main(int argc, char** argv)
{
	C_SDLWindow window(640, 480, "Columbus Engine");
	C_EventSystem event;
	event.addWindow(&window);

	C_Shader shader("shader.vert", "shader.frag");

	C_Texture tex("Textures/wall.jpg");

	std::vector<C_Vertex> verts;
	C_Vertex v1, v2, v3;
	v1.pos = C_Vector3(-0.5, -0.5, 0.0);
	v2.pos = C_Vector3(0.5, -0.5, 0.0);
	v3.pos = C_Vector3(0.0, 0.5, 0.0);
	v1.UV = C_Vector2(0.0, 0.0);
	v2.UV = C_Vector2(1.0, 0.0);
	v3.UV = C_Vector2(0.5, 1.0);

	verts.push_back(v1);
	verts.push_back(v2);
	verts.push_back(v3);


	C_Mesh mesh(verts);
	mesh.mMat.setTexture(&tex);

	C_Camera camera;

	while (window.isOpen())
	{
		event.pollEvents();

		window.clear(1, 1, 1, 1);

		C_SetPerspective(60, window.aspect(), 0.00001, 100000);
		camera.update();

		if(window.getKeyDown(SDL_SCANCODE_A))
			printf("Key down\n");
		if (window.getKeyUp(SDL_SCANCODE_A))
			printf("Key up\n");

		mesh.draw(shader);

		window.display();
		SDL_Delay(16);
	}

	shader.unbind();

	return 0;
}

