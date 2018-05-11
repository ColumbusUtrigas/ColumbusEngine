#include <Engine.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Audio/OpenAL/AudioDeviceOpenAL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>

#include <RenderAPIOpenGL/OpenGL.h>
#include <Graphics/OpenGL/MeshInstancedOpenGL.h>

#include <Audio/AudioPlayer.h>

#include <zstd.h>

using namespace Columbus;

int main(int argc, char** argv)
{
	WindowOpenGLSDL window(Vector2(640, 480), "Columbus Engine", E_WINDOW_FLAG_RESIZABLE);
	
	Input input;
	input.setWindow(&window);

	Camera camera;
	camera.setPos(vec3(10, 10, 0));
	camera.setRot(vec3(0, 90, 0));

	gDevice = new DeviceOpenGL();
	gAudioDevice = new AudioDeviceOpenAL();

	Skybox skybox(gDevice->createCubemap("Data/Skyboxes/1.cubemap"));

	Timer timer;

	window.setVSync(true);

	Image* cur = new Image;
	cur->load("Data/Textures/cursor.tif", E_IMAGE_LOAD_FLIP_Y);

	input.showMouseCursor(false);
	//input.SetSystemCursor(SystemCursor::Crosshair);
	input.SetColoredCursor(cur->getData(), cur->getWidth(), cur->getHeight(), cur->getBPP(), vec2(17, 3));

	bool cursor = false;

	Scene scene;

	scene.load("Data/1.scene");

	scene.setSkybox(&skybox);
	scene.setCamera(&camera);

	AudioSource* Source = gAudioDevice->CreateSource();

	if (!Source->GetSound()->Load("Data/Sounds/cartoon001.ogg"))
	{
		Log::error("Couldn't load sound");
	}

	Source->SetPosition(Vector3(0, 10, 3.5));
	Source->SetLooping(true);
	Source->SetMinDistance(1.0f);
	Source->SetMaxDistance(1000.0f);

	Source->SetSound(Source->GetSound());
	
	//Source->Play();

	float xPos = 8.0f;

	std::cout << "Uniform buffer support: " << (OpenGL::SupportsUniformBuffer() ? "Yes" : "No") << std::endl;
	std::cout << "Instancing support: " << (OpenGL::SupportsInstancing() ? "Yes" : "No") << std::endl;
	std::cout << "Transform feedback support: " << (OpenGL::SupportsTransformFeedback() ? "Yes" : "No") << std::endl;
	std::cout << "Shaders support: " << (OpenGL::SupportsShader() ? "Yes" : "No") << std::endl;
	std::cout << "Geometry shaders support: " << (OpenGL::SupportsGeometryShader() ? "Yes" : "No") << std::endl;
	std::cout << "Tesselation support: " << (OpenGL::SupportsTesselation() ? "Yes" : "No") << std::endl;
	std::cout << "Compute shaders support: " << (OpenGL::SupportsComputeShader() ? "Yes" : "No") << std::endl;

	Material mat;
	ShaderProgram* prog = gDevice->CreateShaderProgram();
	ShaderStage* vert, *frag;
	vert = gDevice->CreateShaderStage();
	frag = gDevice->CreateShaderStage();

	vert->Load("Data/Shaders/instanced.vert", ShaderType::Vertex);
	frag->Load("Data/Shaders/instanced.frag", ShaderType::Fragment);

	prog->AddStage(vert);
	prog->AddStage(frag);

	mat.SetShader(prog);

	MeshInstancedOpenGL mesh;
	mat.setTexture(gDevice->createTexture("Data/Textures/metal.jpg"));
	mat.setSpecMap(gDevice->createTexture("Data/Textures/metal-specular.jpg"));
	mat.setNormMap(gDevice->createTexture("Data/Textures/metal-normal.jpg"));
	mesh.Mat = mat;
	mesh.SetVertices(ModelLoadCMF("Data/Models/Dragon.cmf"));

	/*uint32 fSize = 0;

	char* src;

	FILE* inf = fopen("test.txt", "r");
	fseek(inf, 0, SEEK_END);
	fSize = ftell(inf);
	fseek(inf, 0, SEEK_SET);
	src = new char[fSize];
	fread(src, sizeof(char), fSize, inf);
	fclose(inf);

	uint32 Bound = ZSTD_compressBound(fSize);
	char* dst = new char[Bound];

	uint32 size = ZSTD_compress(dst, Bound, src, fSize, 1);

	std::ofstream ofs;
	ofs.open("test.bin", std::ios::binary);
	ofs.write(dst, size);
	ofs.close();

	delete[] dst;*/

	AudioPlayer player;

	while (window.isOpen())
	{
		player.Play();

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
			camera.addRot(Vector3(-125 * RedrawTime, 0, 0));
		if (input.getKey(SDL_SCANCODE_DOWN))
			camera.addRot(Vector3(125 * RedrawTime, 0, 0));
		if (input.getKey(SDL_SCANCODE_LEFT))
			camera.addRot(Vector3(0, 125 * RedrawTime, 0));
		if (input.getKey(SDL_SCANCODE_RIGHT))
			camera.addRot(Vector3(0, -125 * RedrawTime, 0));

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
			camera.addRot(Vector3(deltaMouse.Y, -deltaMouse.X, 0) * 0.3);
			input.setMousePos(window.getSize() * 0.5);
		}

		xPos -= 0.05f;

		Source->SetPosition(Vector3(xPos, 0.0f, 0.0f));

		camera.setRot(Vector3::Clamp(camera.getRot(), Vector3(-89.9, -360, 0.0), Vector3(89.9, 360, 0.0)));
		camera.update();

		scene.setContextSize(window.getSize());
		scene.update();
		scene.render();

		mesh.SetCamera(camera);
		mesh.Render();

		window.display();

		if ((timer.elapsed()) > 1.0)
		{
			printf("%i\n", window.getFPS());
			timer.reset();
		}
	}

	delete Source;
	delete gDevice;
	delete gAudioDevice;

	return 0;
}


