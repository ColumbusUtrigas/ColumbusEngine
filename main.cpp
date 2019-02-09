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
	gDevice = new DeviceOpenGL();
	Scene scene;
	Input input;
	AudioListener Listener;
	Camera camera;
	Timer timer;

	camera.Pos = Vector3(10, 10, 0);
	camera.Rot = Vector3(0, 180, 0);

	window.SetVSync(true);

	input.ShowMouseCursor(false);
	input.SetSystemCursor(SystemCursor::Crosshair);

	bool cursor = false;
	scene.Load("Data/2.scene");

	scene.SetCamera(&camera);
	scene.SetAudioListener(&Listener);
	scene.Audio.Play();

	Fireplace = scene.GetGameObject(2);
	Fireplace->AddComponent(new FireplaceBright());

	Mesh* Sphere = gDevice->CreateMesh();

	{
		Model SphereModel;
		SphereModel.Load("Data/Models/Sphere.cmf");
		Sphere->Load(SphereModel);
	}

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
			Tests[Roughness * 6 + Metallic].GetMaterial().SetShader(scene.GetGameObject(0)->GetMaterial().GetShader());
			Tests[Roughness * 6 + Metallic].GetMaterial().Albedo = Vector4(1);
			//Tests[Roughness * 6 + Metallic].GetMaterial().Albedo = Vector4(Vector3(0, 1, 0), 0.5);
			//Tests[Roughness * 6 + Metallic].GetMaterial().Transparent = true;
			//Tests[Roughness * 6 + Metallic].GetMaterial().Culling = Material::Cull::No;
			Tests[Roughness * 6 + Metallic].GetMaterial().Roughness = Math::Clamp(Roughness * 0.2f, 0.01f, 1.0f);
			Tests[Roughness * 6 + Metallic].GetMaterial().Metallic = Math::Clamp(Metallic * 0.2f, 0.01f, 1.0f);
			trans.SetPos(Vector3(X, Y, 20));
			Tests[Roughness * 6 + Metallic].SetTransform(trans);
			Tests[Roughness * 6 + Metallic].AddComponent(new ComponentMeshRenderer(Sphere));
			scene.Add(8 + (Roughness * 6 + Metallic), std::move(Tests[Roughness * 6 + Metallic]));
		}
	}

	SDL_Event Event;

	float wheel = 0.0f;

	/*JSON J;
	printf("%i\n", J.Load("test.json"));
	printf("%s\n", J["s"].GetString().c_str());
	printf("%i\n", J["i"].GetInt());
	printf("%f\n", J["f"].GetFloat());
	printf("%i\n", J["b"].GetBool());
	printf("%i\n", J["n"].IsNull());
	for (int i = 0; i < J["a"].GetElementsCount(); i++)
		printf("%i\n", J["a"][i].GetInt());
	printf("%s\n", J["o"]["s"].GetString().c_str());*/
	/*J.Parse("{ \"hello\": \"world\" }");
	printf("%i %i\n", J.IsObject(), J.GetChildrenCount());
	printf("%i %s\n", J["hello"].IsString(), J["hello"].GetString().c_str());*/

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
				input.SetMousePosition({ Event.motion.x, Event.motion.y });
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

		camera.Perspective(60, window.GetAspect(), 0.1f, 1000);

		wheel += input.GetMouseWheel().Y * 5;
		camera.Pos += camera.Direction() * wheel * RedrawTime;
		wheel -= wheel * 3 * RedrawTime;
		if (abs(wheel) <= 0.2) wheel = 0.0f;

		if (input.GetKey(SDL_SCANCODE_W))
			camera.Pos += camera.Direction() * RedrawTime * 5;
		if (input.GetKey(SDL_SCANCODE_S))
			camera.Pos += -camera.Direction() * RedrawTime * 5;
		if (input.GetKey(SDL_SCANCODE_A))
			camera.Pos += -camera.Right() * RedrawTime * 5;
		if (input.GetKey(SDL_SCANCODE_D))
			camera.Pos += camera.Right() * RedrawTime * 5;

		if (input.GetKey(SDL_SCANCODE_LSHIFT))
			camera.Pos += -camera.Up() * RedrawTime * 5;
		if (input.GetKey(SDL_SCANCODE_SPACE))
			camera.Pos += camera.Up() * RedrawTime * 5;
		if (input.GetKey(SDL_SCANCODE_Q))
			camera.Rot += Vector3(0, 0, 125 * RedrawTime);
		if (input.GetKey(SDL_SCANCODE_E))
			camera.Rot += Vector3(0, 0, -125 * RedrawTime);

		if (input.GetKeyDown(SDL_SCANCODE_ESCAPE))
		{
			cursor = !cursor;
			input.ShowMouseCursor(cursor);
		}

		if (!cursor)
		{
			Vector2 deltaMouse = input.GetMouseMovement();
			camera.Rot += Vector3(deltaMouse.Y, -deltaMouse.X, 0) * 0.3f;
			window.SetMousePosition(window.GetSize() / 2);
			input.SetMousePosition (window.GetSize() / 2);
		}

		camera.Rot.Clamp({ -89.9f, -360.0f, 0.0f }, {89.9f, 360.0f, 0.0f});
		camera.Update();

		Listener.Position = camera.Pos;
		Listener.Right = camera.Right();
		Listener.Up = camera.Up();
		Listener.Forward = camera.Direction();

		scene.SetContextSize(window.GetSize());
		scene.Update();
		scene.Render();

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


