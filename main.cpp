#include <Engine.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>

#include <RenderAPIOpenGL/OpenGL.h>

using namespace Columbus;

#ifdef COLUMBUS_PLATFORM_WINDOWS
	//Hint to the driver to use discrete GPU
	extern "C" 
	{
		//NVIDIA
		__declspec(dllexport) int NvOptimusEnablement = 1;
		//AMD
		__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	}
#endif

GameObject* Fireplace = nullptr;

class FireplaceBright : public Component
{
private:
	float Time = 0.0f;
public:
	virtual void Update(float TimeTick, Transform& Trans) override
	{
		float Period = 0.5f;
		float Amplitude = 0.3f;
		float Offset = 0.7f;

		float Sin = Math::Sin(Time += TimeTick / Period);
		Sin = Sin < 0.0f ? (0.0f - Sin) : Sin;
		Fireplace->GetMaterial().EmissionStrength = Sin * Amplitude + Offset;
	}
};

int main(int argc, char** argv)
{
	WindowOpenGLSDL window(Vector2(640, 480), "Columbus Engine", E_WINDOW_FLAG_RESIZABLE);

	Input input;
	VirtualInput VI;

	input.SetWindow(&window);
	input.SetVirtualInput(&VI);

	AudioListener Listener;

	Camera camera;
	camera.setPos(Vector3(10, 10, 0));
	camera.setRot(Vector3(0, 90, 0));

	gDevice = new DeviceOpenGL();

	Scene scene;

	Timer timer;

	window.setVSync(false);

	input.ShowMouseCursor(false);
	input.SetSystemCursor(SystemCursor::Crosshair);

	bool cursor = false;
	scene.load("Data/2.scene");

	scene.setCamera(&camera);
	scene.SetAudioListener(&Listener);
	scene.Audio.Play();

	Fireplace = scene.getGameObject(2);
	Fireplace->AddComponent(new FireplaceBright());

	while (window.isOpen())
	{
		float RedrawTime = window.getRedrawTime();

		window.update();
		input.Update();

		window.clear(Vector4(0, 0, 0.75, 1));

		camera.perspective(60, window.getAspect(), 0.1, 1000);

		if (input.GetKey(SDL_SCANCODE_W))
			camera.addPos(camera.direction() * RedrawTime * 5);
		if (input.GetKey(SDL_SCANCODE_S))
			camera.addPos(-camera.direction() * RedrawTime * 5);
		if (input.GetKey(SDL_SCANCODE_A))
			camera.addPos(-camera.right() * RedrawTime * 5);
		if (input.GetKey(SDL_SCANCODE_D))
			camera.addPos(camera.right() * RedrawTime * 5);

		if (input.GetKey(SDL_SCANCODE_LSHIFT))
			camera.addPos(-camera.up() * RedrawTime * 5);
		if (input.GetKey(SDL_SCANCODE_SPACE))
			camera.addPos(camera.up() * RedrawTime * 5);
		if (input.GetKey(SDL_SCANCODE_Q))
			camera.addRot(Vector3(0, 0, 125 * RedrawTime));
		if (input.GetKey(SDL_SCANCODE_E))
			camera.addRot(Vector3(0, 0, -125 * RedrawTime));

		if (input.GetKeyDown(SDL_SCANCODE_ESCAPE))
		{
			cursor = !cursor;
			input.ShowMouseCursor(cursor);
		}

		if (!cursor)
		{
			Vector2 deltaMouse = input.GetMouseMovement();
			camera.addRot(Vector3(deltaMouse.Y * 60 * RedrawTime, -deltaMouse.X * 60 * RedrawTime, 0) * 0.3);
			input.SetMousePos(window.getSize() * 0.5);
		}

		camera.setRot(Vector3::Clamp(camera.getRot(), Vector3(-89.9, -360, 0.0), Vector3(89.9, 360, 0.0)));
		camera.update();

		Listener.Position = camera.getPos();
		Listener.Right = camera.right();
		Listener.Up = camera.up();
		Listener.Forward = camera.direction();

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

	delete gDevice;

	return 0;
}


