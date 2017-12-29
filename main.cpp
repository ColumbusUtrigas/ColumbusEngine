#include <Engine.h>

using namespace Columbus;

int FPS_LIMIT = 120;


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

	C_Shader shader("Data/Shaders/standart.vert", "Data/Shaders/standart.frag");

	Import::C_ImporterModel imp;
	imp.loadOBJ("Data/Models/ASD.obj");
	Import::C_ImporterModel imp2;
	imp2.loadOBJ("Data/Models/Dragon.obj");


	C_Mesh mesh(imp.getObject(0));
	C_Mesh mesh2(imp.getObject(1));
	C_Mesh mesh3(C_PrimitiveBox(C_Vector3(5, 1, 5)));
	C_Mesh mesh4(imp2.getObject(0));

	mesh.mMat.loadFromXML("Data/Materials/Default.cxmat");
	mesh2.mMat.loadFromXML("Data/Materials/Default.cxmat");
	mesh3.mMat.loadFromXML("Data/Materials/Default.cxmat");
	mesh4.mMat.loadFromXML("Data/Materials/Default.cxmat");

	mesh.mMat.setShader(&shader);
	mesh2.mMat.setShader(&shader);
	mesh3.mMat.setShader(&shader);
	mesh4.mMat.setShader(&shader);

	mesh.addChild(&mesh2);

	C_Camera camera;

	float i = 0;

	C_Cubemap cubemap("Data/Skyboxes/1.cubemap");
	C_Skybox skybox(&cubemap);

	C_Material partmat;
	partmat.loadFromXML("Data/Materials/Smoke.cxmat");
	C_Texture partex("Data/Textures/smoke.png");
	partmat.setTexture(&partex);
	partmat.setColor(C_Vector4(1.0, 1.0, 1.0, 0.5));


	C_ParticleEffect particleEffect;
	particleEffect.loadFromXML("Data/Particles/A.cxpar");
	particleEffect.setMaterial(&partmat);
	C_ParticleEmitter particles(&particleEffect);

	C_Timer timer;

	int FPS = 0;

	//window.setVerticalSync(true);
	window.setFPSLimit(100);

	C_Light light1("Data/Lights/1.light");
	C_Light light2("Data/Lights/2.light");
	C_Light light3("Data/Lights/3.light");

	input.showMouseCursor(false);

	bool cursor = false;

	C_Scene scene;

	scene.load("Data/1.scene");

	scene.setSkybox(&skybox);
	scene.setCamera(&camera);

	C_GameObject* obj = new C_GameObject();
	C_GameObject* obj2 = new C_GameObject();
	C_GameObject* part = new C_GameObject();
	C_GameObject* l1 = new C_GameObject();
	C_GameObject* l2 = new C_GameObject();
	C_GameObject* l3 = new C_GameObject();

	obj->addComponent(new C_MeshRenderer(&mesh3));
	obj2->addComponent(new C_MeshRenderer(&mesh4));
	part->addComponent(new C_ParticleSystem(&particles));
	l1->addComponent(new C_LightComponent(&light1));
	l2->addComponent(new C_LightComponent(&light2));
	l3->addComponent(new C_LightComponent(&light3));

	C_Transform transf;
	transf.setPos(C_Vector3(0, -2, 0));
	obj->setTransform(transf);
	transf.setPos(C_Vector3(0, -1, 0));
	obj2->setTransform(transf);

	scene.add(0, obj);
	scene.add(1, obj2);
	scene.add(2, part);
	scene.add(3, l1);
	scene.add(4, l2);
	scene.add(5, l3);
	
	while (window.isOpen())
	{
		float RedrawTime = window.getRedrawTime();

		event.pollEvents();
		input.update();

		window.clear(0, 0, 0.75, 1);

		C_SetPerspective(60, window.aspect(), 0.001, 1000);

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

		if (input.getKey(SDL_SCANCODE_I))
			particleEffect.addPos(C_Vector3(0, 0, -3) * RedrawTime);
		if (input.getKey(SDL_SCANCODE_K))
			particleEffect.addPos(C_Vector3(0, 0, 3) * RedrawTime);
		if (input.getKey(SDL_SCANCODE_J))
			particleEffect.addPos(C_Vector3(-3, 0, 0) * RedrawTime);
		if (input.getKey(SDL_SCANCODE_L))
			particleEffect.addPos(C_Vector3(3, 0, 0) * RedrawTime);

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


