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
	WindowOpenGLSDL window({ 640, 480 }, "Columbus Engine", Window::Flags::Resizable);
	Input input;
	AudioListener Listener;

	Camera camera;
	camera.setPos(Vector3(10, 10, 0));
	camera.setRot(Vector3(0, 180, 0));

	gDevice = new DeviceOpenGL();

	Scene scene;

	Timer timer;

	window.SetVSync(false);

	input.ShowMouseCursor(false);
	input.SetSystemCursor(SystemCursor::Crosshair);

	bool cursor = false;
	scene.load("Data/2.scene");

	scene.SetCamera(&camera);
	scene.SetAudioListener(&Listener);
	scene.Audio.Play();

	Fireplace = scene.getGameObject(2);
	Fireplace->AddComponent(new FireplaceBright());

	Mesh* Sphere = gDevice->CreateMesh();

	{
		Model SphereModel;
		SphereModel.Load("Data/Models/Sphere.cmf");
		Sphere->Load(SphereModel);
	}

	float Values[] = { 0.01f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f };

	GameObject Tests[36];

	float X = -6.0f;
	float Y = 6.0f;

	Transform trans;

	for (int Roughness = 0; Roughness < 6; Roughness++)
	{
		Y = 6.0f;
		X += 2;

		for (int Metallic = 0; Metallic < 6; Metallic++)
		{
			Y -= 2;
			Tests[Roughness * 6 + Metallic].GetMaterial().SetShader(scene.getGameObject(0)->GetMaterial().GetShader());
			Tests[Roughness * 6 + Metallic].GetMaterial().Color = Vector4(1);
			//Tests[Roughness * 6 + Metallic].GetMaterial().Color = Vector4(Vector3(0, 1, 0), 0.5);
			//Tests[Roughness * 6 + Metallic].GetMaterial().Transparent = true;
			//Tests[Roughness * 6 + Metallic].GetMaterial().Culling = Material::Cull::No;
			Tests[Roughness * 6 + Metallic].GetMaterial().Roughness = Values[Roughness];
			Tests[Roughness * 6 + Metallic].GetMaterial().Metallic = Values[Metallic];
			trans.SetPos(Vector3(X, Y, 20));
			Tests[Roughness * 6 + Metallic].SetTransform(trans);
			Tests[Roughness * 6 + Metallic].AddComponent(new ComponentMeshRenderer(Sphere));
			scene.Add(8 + (Roughness * 6 + Metallic), std::move(Tests[Roughness * 6 + Metallic]));
		}
	}

	SDL_Event Event;

	float wheel = 0.0f;

	while (window.IsOpen())
	{
		float RedrawTime = window.GetRedrawTime();

		input.Update();

		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_QUIT:    window.Close();                              break;
			case SDL_KEYDOWN: input.SetKeyDown(Event.key.keysym.scancode); break;
			case SDL_KEYUP:   input.SetKeyUp(Event.key.keysym.scancode);   break;
			case SDL_MOUSEMOTION:
				input.SetMousePosition({ Event.motion.x,    Event.motion.y });
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				input.SetMouseButton(Event.button.button, { Event.button.x, Event.button.y, (bool)Event.button.state, Event.button.clicks });
				break;
			case SDL_MOUSEWHEEL: input.SetMouseWheel({ Event.wheel.x, Event.wheel.y }); break;
			}

			window.PollEvent(Event);
		}

		window.Update();
		window.Clear({ 0, 0, 0.75f, 1 });

		input.SetKeyboardFocus(window.HasKeyFocus());
		input.SetMouseFocus(window.HasMouseFocus());

		camera.perspective(60, window.GetAspect(), 0.1f, 1000);

		wheel += input.GetMouseWheel().Y * 5;
		camera.addPos(camera.direction() * wheel * RedrawTime);
		wheel -= wheel * 3 * RedrawTime;
		if (abs(wheel) <= 0.2) wheel = 0.0f;

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
			camera.addRot(Vector3(deltaMouse.Y, -deltaMouse.X, 0) * 0.3f);
			window.SetMousePosition(window.GetSize() / 2);
			input.SetMousePosition (window.GetSize() / 2);
		}

		camera.setRot(Vector3::Clamp(camera.getRot(), Vector3(-89.9f, -360.0f, 0.0f), Vector3(89.9f, 360.0f, 0.0f)));
		camera.update();

		Listener.Position = camera.getPos();
		Listener.Right = camera.right();
		Listener.Up = camera.up();
		Listener.Forward = camera.direction();

		scene.SetContextSize(window.GetSize());
		scene.update();
		scene.render();

		window.Display();

		if (timer.Elapsed() > 1.0)
		{
			printf("%i\n", window.GetFPS());
			timer.Reset();
		}
	}

	delete gDevice;
	delete Sphere;

	return 0;
}


