#pragma once

#include <Common/Compress/Compress.h>
#include <Common/Image/Image.h>
#include <Common/Model/Model.h>
#include <Common/Noise/OctaveNoise.h>
#include <Common/Noise/PerlinNoise.h>

#include <Graphics/Camera.h>
#include <Graphics/Cubemap.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Light.h>
#include <Graphics/Material.h>
#include <Graphics/Mesh.h>
#include <Graphics/ParticleEffect.h>
#include <Graphics/ParticleEmitter.h>
#include <Graphics/PostEffect.h>
#include <Graphics/Primitives.h>
#include <Graphics/Render.h>
#include <Graphics/Renderbuffer.h>
#include <Graphics/Shader.h>
#include <Graphics/ShaderBuilder.h>
#include <Graphics/Skybox.h>
#include <Graphics/Texture.h>
#include <Graphics/TextureManager.h>

#include <GUI/Button.h>
#include <GUI/IO.h>
#include <GUI/Widget.h>

#include <Impl/EventSystem.h>
#include <Impl/ImplSDL.h>

#include <Input/Input.h>

#include <Math/MathUtil.h>
#include <Math/Matrix4.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

#include <RenderAPI/APIOpenGL.h>
#include <RenderAPI/Buffer.h>

#include <Scene/Component.h>
#include <Scene/GameObject.h>
#include <Scene/LightComponent.h>
#include <Scene/MeshRenderer.h>
#include <Scene/ParticleSystem.h>
#include <Scene/Scene.h>
#include <Scene/Transform.h>

#include <System/Assert.h>
#include <System/File.h>
#include <System/Importer.h>
#include <System/Log.h>
#include <System/Random.h>
#include <System/ResourceManager.h>
#include <System/Serializer.h>
#include <System/SmartPointer.h>
#include <System/System.h>
#include <System/Timer.h>
