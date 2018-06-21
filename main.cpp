#include <Engine.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Audio/OpenAL/AudioDeviceOpenAL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>

#include <RenderAPIOpenGL/OpenGL.h>
#include <Graphics/OpenGL/MeshInstancedOpenGL.h>

#include <Audio/AudioSystem.h>

#include <SDL_ttf.h>
#include <minimp3.h>

using namespace Columbus;

#ifdef COLUMBUS_PLATFORM_WINDOWS
	// hint to the driver to use discrete GPU
	extern "C" 
	{
		// NVIDIA
		__declspec(dllexport) int NvOptimusEnablement = 1;
		// AMD
		__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	}
#endif

class Rotator : public Component
{
private:
	Vector3 v;
public:
	void Update(float TimeTick) override
	{
		v += Vector3(0, 45, 0) * TimeTick;
	}

	void Render(Transform& Trans) override
	{
		Trans.SetRot(v);
	}
};

int main(int argc, char** argv)
{
	WindowOpenGLSDL window(Vector2(640, 480), "Columbus Engine", E_WINDOW_FLAG_RESIZABLE);
	
	Input input;
	input.setWindow(&window);

	AudioSystem Audio;
	AudioListener Listener;

	Camera camera;
	camera.setPos(vec3(10, 10, 0));
	camera.setRot(vec3(0, 90, 0));

	gDevice = new DeviceOpenGL();
	gAudioDevice = new AudioDeviceOpenAL();

	Skybox skybox(gDevice->createCubemap("Data/Skyboxes/1.cubemap"));

	Timer timer;

	window.setVSync(false);

	Image CursorImage;
	CursorImage.Load("Data/Textures/cursor.tga", ImageLoading::FlipY);

	input.showMouseCursor(false);
	input.SetSystemCursor(SystemCursor::Crosshair);

	Cursor Cursor(CursorImage, 3, 3);
	input.SetCursor(Cursor);

	bool cursor = false;

	Scene scene;

	scene.load("Data/1.scene");

	scene.setSkybox(&skybox);
	scene.setCamera(&camera);

	AudioSource* Source1 = new AudioSource();
	AudioSource* Source2 = new AudioSource();
	AudioSource* BackgroundMusic = new AudioSource();

	Sound FireSound;
	Sound BackgroundSound;
	FireSound.Load("Data/Sounds/Fire.ogg");
	BackgroundSound.Load("Data/Sounds/thestonemasons.ogg", true);

	BackgroundMusic->SetSound(&BackgroundSound);
	BackgroundMusic->SetMode(AudioSource::Mode::Sound2D);
	BackgroundMusic->SetLooping(true);
	BackgroundMusic->Play();

	Source1->SetSound(&FireSound);
	Source2->SetSound(&FireSound);

	Source1->SetPlayedTime(Random::range(0.0f, FireSound.GetLength()));
	Source2->SetPlayedTime(Random::range(0.0f, FireSound.GetLength()));

	Source1->SetPosition(Vector3(0, 10, 3.5));
	Source2->SetPosition(Vector3(0, 10, -3.5));

	Source1->SetLooping(true);
	Source2->SetLooping(true);
	Source1->Play();
	Source2->Play();

	Audio.AddSource(BackgroundMusic);
	Audio.AddSource(Source1);
	Audio.AddSource(Source2);

	scene.getGameObject(12)->AddComponent(new Rotator());

	auto Sphere = scene.getGameObject(15);
	Rigidbody* RB = static_cast<ComponentRigidbody*>(Sphere->GetComponent(Component::Type::Rigidbody))->GetRigidbody();

	/*GameObject Tests[3000];

	for (uint32 i = 0; i < 3000; i++)
	{
		Tests[i].AddComponent(Sphere->GetComponent(Component::Type::MeshRenderer));
		Tests[i].SetTransform(Transform(Vector3((float)i * 0.1, 0, 0)));
		scene.Add(21 + i, std::move(Tests[i]));
	}*/

	Audio.Play();

	TTF_Init();
	auto Font = TTF_OpenFont("Data/A.ttf", 35);

	SDL_Color Color;
	Color.r = 255;
	Color.g = 0;
	Color.b = 0;
	Color.a = 255;

	SDL_Surface* Surf = TTF_RenderUNICODE_Blended(Font, (Uint16*)L"ÍÊÂÄ", Color);

	ImageBGRA2RGBA((uint8*)Surf->pixels, Surf->w * Surf->h * 4);
	ImageFlipY((uint8*)Surf->pixels, Surf->w, Surf->h, 4);

	Texture* FontTexture = gDevice->CreateTexture();
	FontTexture->Create2D(Texture::Properties(Surf->w, Surf->h, 1, 0, 0, TextureFormat::RGBA8));
	FontTexture->Load(Surf->pixels);
	FontTexture->SetFlags(Texture::Flags{ Texture::Filter::Point, Texture::Anisotropy::Anisotropy8 });
	static_cast<ComponentMeshRenderer*>(scene.getGameObject(19)->GetComponent(Component::Type::MeshRenderer))->GetMesh()->mMat.setTexture(FontTexture);

	while (window.isOpen())
	{
		float RedrawTime = window.getRedrawTime();

		window.update();
		input.update();

		window.clear(vec4(0, 0, 0.75, 1));

		camera.perspective(60, window.getAspect(), 0.1, 1000);

		if (input.getKey(SDL_SCANCODE_W))
			camera.addPos(camera.direction() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_S))
			camera.addPos(-camera.direction() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_A))
			camera.addPos(-camera.right() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_D))
			camera.addPos(camera.right() * RedrawTime * 5);

		if (input.getKey(SDL_SCANCODE_UP))
			RB->ApplyCentralImpulse(Vector3(-0.3, 0, 0) * 60 * RedrawTime);
		if (input.getKey(SDL_SCANCODE_DOWN))
			RB->ApplyCentralImpulse(Vector3(0.3, 0, 0) * 60 * RedrawTime);
		if (input.getKey(SDL_SCANCODE_LEFT))
			RB->ApplyCentralImpulse(Vector3(0, 0, 0.3) * 60 * RedrawTime);
		if (input.getKey(SDL_SCANCODE_RIGHT))
			RB->ApplyCentralImpulse(Vector3(0, 0, -0.3) * 60 * RedrawTime);

		if (input.getKey(SDL_SCANCODE_LSHIFT))
			camera.addPos(-camera.up() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_SPACE))
			camera.addPos(camera.up() * RedrawTime * 5);
		if (input.getKey(SDL_SCANCODE_Q))
			camera.addRot(Vector3(0, 0, 125 * RedrawTime));
		if (input.getKey(SDL_SCANCODE_E))
			camera.addRot(Vector3(0, 0, -125 * RedrawTime));

		if (input.getKeyDown(SDL_SCANCODE_ESCAPE))
		{
			cursor = !cursor;
			input.showMouseCursor(cursor);
		}

		if (!cursor)
		{
			Vector2 deltaMouse = input.getMouseMovement();
			camera.addRot(Vector3(deltaMouse.Y * 60 * RedrawTime, -deltaMouse.X * 60 * RedrawTime, 0) * 0.3);
			input.setMousePos(window.getSize() * 0.5);
		}

		camera.setRot(Vector3::Clamp(camera.getRot(), Vector3(-89.9, -360, 0.0), Vector3(89.9, 360, 0.0)));
		camera.update();

		Listener.Position = camera.getPos();
		Listener.Right = camera.right();
		Listener.Up = camera.up();
		Listener.Forward = camera.direction();
		Audio.SetListener(Listener);

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

	//delete Source;
	delete gDevice;
	delete gAudioDevice;

	return 0;
}


