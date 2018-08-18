#include <Engine.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>

#include <RenderAPIOpenGL/OpenGL.h>

#include <SDL_ttf.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

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

class Rotator : public Component
{
private:
	Vector3 v;
public:
	void Update(float TimeTick, Transform& Trans) override
	{
		v += Vector3(0, 45, 0) * TimeTick;
		Trans.SetRot(v);
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

	Image ReflImage;
	ReflImage.Load("./Data/Skyboxes/Sky.dds");

	Texture* Refl = gDevice->CreateTexture();
	Refl->CreateCube(Texture::Properties(ReflImage.GetWidth(), ReflImage.GetHeight(), 0, ReflImage.GetFormat()));
	Refl->Load(ReflImage);

	Skybox skybox(Refl);

	Timer timer;

	window.setVSync(false);

	Image CursorImage;
	CursorImage.Load("Data/Textures/cursor.tga", ImageLoading::FlipY);

	input.ShowMouseCursor(false);
	input.SetSystemCursor(SystemCursor::Crosshair);

	Cursor Cursor(CursorImage, 3, 3);
	input.SetCursor(Cursor);

	bool cursor = false;

	Image BImage;
	BImage.Load("Data/Textures/Button.tga");

	auto BTexture = gDevice->CreateTexture();
	BTexture->Create2D(Texture::Properties(BImage.GetWidth(), BImage.GetHeight(), 0, BImage.GetFormat()));
	BTexture->Load(BImage);

	GUI UI;
	Button B({0, 0}, {1, 0.4});

	B.NoneColor = {1, 0, 0, 0.8};
	B.HoverColor = {0, 1, 0, 0.8};
	B.DownColor = {0, 0, 1, 0.8};
	B.Shader = gDevice->CreateShaderProgram();
	B.Shader->Load("Data/Shaders/GUI.glsl");
	B.MainTexture = BTexture;
	UI.Add(&B);

	scene.load("Data/1.scene");

	scene.setSkybox(&skybox);
	scene.setCamera(&camera);
	scene.SetAudioListener(&Listener);

	AudioSource* Source1 = new AudioSource();
	AudioSource* Source2 = new AudioSource();
	AudioSource* BackgroundMusic = new AudioSource();

	Sound FireSound;
	Sound BackgroundSound;
	FireSound.Load("Data/Sounds/Fire.ogg");
	BackgroundSound.Load("Data/Sounds/thestonemasons.wav", true);

	BackgroundMusic->SetSound(&BackgroundSound);
	BackgroundMusic->SetMode(AudioSource::Mode::Sound2D);
	BackgroundMusic->SetLooping(true);
	BackgroundMusic->Play();

	Source1->SetSound(&FireSound);
	Source2->SetSound(&FireSound);

	Source1->SetPlayedTime(Random::Range(0.0, FireSound.GetLength()));
	Source2->SetPlayedTime(Random::Range(0.0, FireSound.GetLength()));

	Source1->SetPosition(Vector3(0, 10, 3.5));
	Source2->SetPosition(Vector3(0, 10, -3.5));

	Source1->SetLooping(true);
	Source2->SetLooping(true);
	Source1->Play();
	Source2->Play();

	scene.Audio.AddSource(BackgroundMusic);
	scene.Audio.AddSource(Source1);
	scene.Audio.AddSource(Source2);
	scene.Audio.Play();

	scene.getGameObject(12)->AddComponent(new Rotator());

	auto Sphere = scene.getGameObject(15);
	Rigidbody* RB = static_cast<ComponentRigidbody*>(Sphere->GetComponent(Component::Type::Rigidbody))->GetRigidbody();

	/*constexpr int TestsSize = 500;
	GameObject Tests[TestsSize];

	for (uint32 i = 0; i < TestsSize; i++)
	{
		Tests[i].AddComponent(Sphere->GetComponent(Component::Type::MeshRenderer));
		Tests[i].SetTransform(Transform(Vector3((float)i * 0.1, 0, 0)));
		Tests[i].SetMaterial(Sphere->GetMaterial());
		scene.Add(21 + i, std::move(Tests[i]));
	}*/

	Animation anim;
	anim.PositionAnimation.AddPoint({0, 0.2, 0}, 0.0);
	anim.PositionAnimation.AddPoint({1, 0.2, 1}, 0.25);
	anim.PositionAnimation.AddPoint({2, 0.2, 0}, 0.5);
	anim.PositionAnimation.AddPoint({1, 0.2, -1}, 0.75);
	anim.PositionAnimation.AddPoint({0, 0.2, 0}, 1.0);
	anim.Speed = 0.5;

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

		if (input.GetKey(SDL_SCANCODE_UP))
			RB->ApplyCentralImpulse(Vector3(-0.3, 0, 0) * 60 * RedrawTime);
		if (input.GetKey(SDL_SCANCODE_DOWN))
			RB->ApplyCentralImpulse(Vector3(0.3, 0, 0) * 60 * RedrawTime);
		if (input.GetKey(SDL_SCANCODE_LEFT))
			RB->ApplyCentralImpulse(Vector3(0, 0, 0.3) * 60 * RedrawTime);
		if (input.GetKey(SDL_SCANCODE_RIGHT))
			RB->ApplyCentralImpulse(Vector3(0, 0, -0.3) * 60 * RedrawTime);

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

		anim.Update(RedrawTime);
		scene.getGameObject(12)->GetTransform().SetPos(anim.GetCurrentPosition());

		scene.setContextSize(window.getSize());
		scene.update();
        scene.render();
        UI.Render(VI);

		window.display();

		if ((timer.elapsed()) > 1.0)
		{
			printf("%i\n", window.getFPS());
			timer.reset();
		}
	}

	//delete Source;
	delete gDevice;

	return 0;
}


