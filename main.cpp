#include <Engine.h>
#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/WindowOpenGLSDL.h>

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

	Skybox skybox(gDevice->createCubemap("Data/Skyboxes/1.cubemap"));

	Timer timer;

	window.setVSync(true);

	Image* cur = new Image("Data/Textures/cursor.tif", E_IMAGE_LOAD_FLIP_Y);

	input.showMouseCursor(false);
	//input.setSystemCursor(E_INPUT_SYSTEM_CURSOR_CROSSHAIR);
	input.setColoredCursor(cur->getData(), cur->getWidth(), cur->getHeight(), cur->getBPP(), vec2(17, 3));

	bool cursor = false;

	Scene scene;

	scene.load("Data/1.scene");

	scene.setSkybox(&skybox);
	scene.setCamera(&camera);

	PhysicsWorld world;
	world.SetGravity(Vector3(0, -9.81, 0));

	PhysicsShapeBox BoxShape(Vector3(100, 0.2, 100));
	Rigidbody Plane(Transform(Vector3(0, 0, 0)), &BoxShape);
	Plane.SetMass(0);
	world.AddRigidbody(&Plane);

	PhysicsShapeSphere SphereShape(1);
	Rigidbody rigidbody(Transform(Vector3(0, 15, 0)), &SphereShape);
	rigidbody.SetMass(1);
	rigidbody.SetFriction(0.8);
	rigidbody.SetLinearVelocity(Vector3(0, 0, 5));
	rigidbody.SetLinearDamping(0.3);
	rigidbody.SetAngularDamping(0.3);
	world.AddRigidbody(&rigidbody);

	ShaderOpenGL* shader = new ShaderOpenGL("Data/Shaders/standart.vert", "Data/Shaders/standart.frag");
	TextureOpenGL* tex = new TextureOpenGL("Data/Textures/metal.jpg");
	Material* mat = new Material();
	mat->setTexture(tex);
	mat->setShader(shader);
	MeshOpenGL* mesh = new MeshOpenGL(ModelLoadCMF("Data/Models/Sphere.cmf"), *mat);
	
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
			camera.addRot(Vector3(deltaMouse.y, -deltaMouse.x, 0) * 0.3);
			input.setMousePos(window.getSize() * 0.5);
		}

		camera.setRot(Vector3::clamp(camera.getRot(), Vector3(-89.9, -360, 0.0), Vector3(89.9, 360, 0.0)));
		camera.update();

		scene.setContextSize(window.getSize());
		scene.update();
		scene.render();

		world.Step(RedrawTime, 10);

		mesh->setCamera(camera);
		mesh->render(rigidbody.GetTransform());

		window.display();

		if ((timer.elapsed()) > 1.0)
		{
			printf("%i\n", window.getFPS());
			timer.reset();
		}
	}

	world.RemoveRigidbody(&rigidbody);
	world.RemoveRigidbody(&Plane);

	return 0;
}


