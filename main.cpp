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
	C_Vertex v[36];

	v[0].pos = C_Vector3(-1.0f, -1.0f, -1.0f);
	v[0].UV = C_Vector2(0, 0);
	v[1].pos = C_Vector3(-1.0f, -1.0f, 1.0f);
	v[1].UV = C_Vector2(0, 1);
	v[2].pos = C_Vector3(-1.0f, 1.0f, 1.0f);
	v[2].UV = C_Vector2(1, 1);
	v[3].pos = C_Vector3(1.0f, 1.0f, -1.0f);
	v[3].UV = C_Vector2(1, 1);
	v[4].pos = C_Vector3(-1.0f, -1.0f, -1.0f);
	v[4].UV = C_Vector2(0, 0);
	v[5].pos = C_Vector3(-1.0f, 1.0f, -1.0f);
	v[5].UV = C_Vector2(0, 1);
	v[6].pos = C_Vector3(1.0f, -1.0f, 1.0f);
	v[6].UV = C_Vector2(1, 1);
	v[7].pos = C_Vector3(-1.0f, -1.0f, -1.0f);
	v[7].UV = C_Vector2(0, 0);
	v[8].pos = C_Vector3(1.0f, -1.0f, -1.0f);
	v[8].UV = C_Vector2(1, 0);
	v[9].pos = C_Vector3(1.0f, 1.0f, -1.0f);
	v[9].UV = C_Vector2(1, 1);
	v[10].pos = C_Vector3(1.0f, -1.0f, -1.0f);
	v[10].UV = C_Vector2(1, 0);
	v[11].pos = C_Vector3(-1.0f, -1.0f, -1.0f);
	v[11].UV = C_Vector2(0, 0);
	v[12].pos = C_Vector3(-1.0f, -1.0f, -1.0f);
	v[12].UV = C_Vector2(0,0);
	v[13].pos = C_Vector3(-1.0f, 1.0f, 1.0f);
	v[13].UV = C_Vector2(1, 1);
	v[14].pos = C_Vector3(-1.0f, 1.0f, -1.0f);
	v[14].UV = C_Vector2(1, 0);
	v[15].pos = C_Vector3(1.0f, -1.0f, 1.0f);
	v[15].UV = C_Vector2(1, 1);
	v[16].pos = C_Vector3(-1.0f, -1.0f, 1.0f);
	v[16].UV = C_Vector2(0, 1);
	v[17].pos = C_Vector3(-1.0f, -1.0f, -1.0f);
	v[17].UV = C_Vector2(0, 0);
	v[18].pos = C_Vector3(-1.0f, 1.0f, 1.0f);
	v[18].UV = C_Vector2(0, 1);
	v[19].pos = C_Vector3(-1.0f, -1.0f, 1.0f);
	v[19].UV = C_Vector2(0, 0);
	v[20].pos = C_Vector3(1.0f, -1.0f, 1.0f);
	v[20].UV = C_Vector2(1, 0);
	v[21].pos = C_Vector3(1.0f, 1.0f, 1.0f);
	v[21].UV = C_Vector2(1, 1);
	v[22].pos = C_Vector3(1.0f, -1.0f, -1.0f);
	v[22].UV = C_Vector2(0, 0);
	v[23].pos = C_Vector3(1.0f, 1.0f, -1.0f);
	v[23].UV = C_Vector2(1, 0);
	v[24].pos = C_Vector3(1.0f, -1.0f, -1.0f);
	v[24].UV = C_Vector2(0, 0);
	v[25].pos = C_Vector3(1.0f, 1.0f, 1.0f);
	v[25].UV = C_Vector2(1, 1);
	v[26].pos = C_Vector3(1.0f, -1.0f, 1.0f);
	v[26].UV = C_Vector2(0, 1);
	v[27].pos = C_Vector3(1.0f, 1.0f, 1.0f);
	v[27].UV = C_Vector2(1, 1);
	v[28].pos = C_Vector3(1.0f, 1.0f, -1.0f);
	v[28].UV = C_Vector2(1, 0);
	v[29].pos = C_Vector3(-1.0f, 1.0f, -1.0f);
	v[29].UV = C_Vector2(0, 0);
	v[30].pos = C_Vector3(1.0f, 1.0f, 1.0f);
	v[30].UV = C_Vector2(1, 1);
	v[31].pos = C_Vector3(-1.0f, 1.0f, -1.0f);
	v[31].UV = C_Vector2(0, 0);
	v[32].pos = C_Vector3(-1.0f, 1.0f, 1.0f);
	v[32].UV = C_Vector2(0, 1);
	v[33].pos = C_Vector3(1.0f, 1.0f, 1.0f);
	v[33].UV = C_Vector2(1, 1);
	v[34].pos = C_Vector3(-1.0f, 1.0f, 1.0f);
	v[34].UV = C_Vector2(0, 1);
	v[35].pos = C_Vector3(1.0f, -1.0f, 1.0f);
	v[35].UV = C_Vector2(1, 0);

	for(int i = 0; i < 36; i++)
		verts.push_back(v[i]);
	C_Mesh mesh(verts);
	mesh.mMat.setTexture(&tex);
	//mesh.mMat.setColor(C_Vector4(1, 1, 0, 1));

	C_Camera camera;

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

		mesh.draw(shader);

		window.display();

		SDL_Delay(16);
	}

	shader.unbind();

	return 0;
}

