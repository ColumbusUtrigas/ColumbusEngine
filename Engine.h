#pragma once

#include <Animation/Animation.h>

#include <Audio/AudioListener.h>
#include <Audio/AudioPlayer.h>
#include <Audio/AudioSource.h>
#include <Audio/AudioSystem.h>

#include <Common/Cursor/Cursor.h>
#include <Common/Image/Image.h>
#include <Common/Model/Model.h>
#include <Common/Noise/OctaveNoise.h>
#include <Common/Noise/PerlinNoise.h>
#include <Common/Sound/Sound.h>

#include <Core/Core.h>

#include <Graphics/Camera.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Light.h>
#include <Graphics/Material.h>
#include <Graphics/Mesh.h>
#include <Graphics/Particles/ParticleEffect.h>
#include <Graphics/Particles/ParticleEmitter.h>
#include <Graphics/PostEffect.h>
#include <Graphics/Primitives.h>
#include <Graphics/Render.h>
#include <Graphics/Shader.h>
#include <Graphics/ShaderBuilder.h>
#include <Graphics/Skybox.h>
#include <Graphics/Texture.h>
#include <Graphics/UIRender.h>
#include <Graphics/Window.h>

#include <GUI/Button.h>
#include <GUI/GUI.h>
#include <GUI/VirtualInput.h>
#include <GUI/Widget.h>

#include <Input/Input.h>

#include <Math/Frustum.h>
#include <Math/MathUtil.h>
#include <Math/OBB.h>
#include <Math/Quaternion.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

#include <Physics/PhysicsShape.h>
#include <Physics/PhysicsShapeBox.h>
#include <Physics/PhysicsShapeCapsule.h>
#include <Physics/PhysicsShapeCone.h>
#include <Physics/PhysicsShapeConvexHull.h>
#include <Physics/PhysicsShapeCylinder.h>
#include <Physics/PhysicsShapeMultiSphere.h>
#include <Physics/PhysicsShapeSphere.h>
#include <Physics/PhysicsWorld.h>
#include <Physics/Rigidbody.h>

#include <Scene/Component.h>
#include <Scene/GameObject.h>
#include <Scene/ComponentLight.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/Scene.h>
#include <Scene/Transform.h>

#include <System/Assert.h>
#include <System/File.h>
#include <System/Log.h>
#include <System/Serializer.h>
#include <System/Timer.h>
