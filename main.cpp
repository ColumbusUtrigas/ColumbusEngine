#include <Engine.h>

using namespace Columbus;

int main(int argc, char** argv)
{
	C_SDLWindowConfig config;
	config.Resizable = true;
	C_SDLWindow window(config);
	C_EventSystem event;
	event.addWindow(&window);
	GUI::C_IO io;
	C_Input input;
	input.setWindow(&window);
	input.setIO(&io);

	C_Camera camera;
	camera.setPos(vec3(10, 10, 0));
	camera.setRot(vec3(0, 90, 0));

	float i = 0;

	C_Cubemap cubemap("Data/Skyboxes/1.cubemap");
	C_Skybox skybox(&cubemap);

	C_Timer timer;

	int FPS = 0;

	//window.setVerticalSync(true);
	window.setFPSLimit(60);

	C_Image* cur = new C_Image("Data/Textures/cursor.tif", E_IMAGE_LOAD_FLIP_Y);

	input.showMouseCursor(false);
	//input.setSystemCursor(E_INPUT_SYSTEM_CURSOR_NO);
	input.setColoredCursor(cur->getData(), cur->getWidth(), cur->getHeight(), cur->getBPP(), vec2(17, 3));

	bool cursor = false;

	C_Scene scene;

	scene.load("Data/1.scene");

	scene.setSkybox(&skybox);
	scene.setCamera(&camera);
	
	while (window.isOpen())
	{
		float RedrawTime = window.getRedrawTime();

		event.pollEvents();
		input.update();

		window.clear(0, 0, 0.75, 1);

		C_SetPerspective(60, window.aspect(), 0.1, 1000);

		if (input.getKey(SDL_SCANCODE_W))
			camera.addPos(camera.direction() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_S))
			camera.addPos(-camera.direction() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_A))
			camera.addPos(-camera.right() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_D))
			camera.addPos(camera.right() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_UP))
			camera.addRot(C_Vector3(-125 * RedrawTime, 0, 0));
		if (input.getKey(SDL_SCANCODE_DOWN))
			camera.addRot(C_Vector3(125 * RedrawTime, 0, 0));
		if (input.getKey(SDL_SCANCODE_LEFT))
			camera.addRot(C_Vector3(0, 125 * RedrawTime, 0));
		if (input.getKey(SDL_SCANCODE_RIGHT))
			camera.addRot(C_Vector3(0, -125 * RedrawTime, 0));

		if (input.getKey(SDL_SCANCODE_LSHIFT))
			camera.addPos(-camera.up() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_SPACE))
			camera.addPos(camera.up() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_Q))
			camera.addRot(C_Vector3(0, 0, 125 * RedrawTime));
		if (input.getKey(SDL_SCANCODE_E))
			camera.addRot(C_Vector3(0, 0, -125 * RedrawTime));

		if (input.getKeyDown(SDL_SCANCODE_ESCAPE))
		{
			cursor = !cursor;
			input.showMouseCursor(cursor);
		}

		if (!cursor)
		{
			C_Vector2 deltaMouse = input.getMouseMovement();
			camera.addRot(C_Vector3(deltaMouse.y, -deltaMouse.x, 0) * 0.3);
			input.setMousePos(window.getSize() * 0.5);
		}

		camera.setRot(C_Vector3::clamp(camera.getRot(), C_Vector3(-89.9, -360, 0.0), C_Vector3(89.9, 360, 0.0)));
		camera.update();

		scene.setContextSize(window.getSize());
		scene.update();
		scene.render();

		window.display();

		if ((timer.elapsed()) > 1.0)
		{
			printf("%i\n", window.getFPS());
			timer.reset();
		}
	}

	return 0;
}


