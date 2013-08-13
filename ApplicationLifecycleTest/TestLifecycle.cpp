#include "TestLifecycle.h"

#include <Platform/Application.h>
#include <Utils/Logger.h>
#include <Graphics/GraphicsSystem.h>

#if defined(LINUX)
	#include <Graphics/Platform/Linux/LinuxRenderPlatform.h>
#elif defined(PANDORA)
	#include <Graphics/Platform/Pandora/PandoraRenderPlatform.h>
#elif defined(ANDROID)
	#include <Graphics/Platform/Android/AndroidRenderPlatform.h>
#elif defined(IOS)
	#include <Graphics/Platform/iOS/iOSRenderPlatform.h>
#endif

#include <Events/EventSystem.h>
#include <Input/InputSystem.h>
#include <Input/Keyboard.h>

#include <Graphics/Camera.h>
#include <Graphics/Mesh.h>
#include <Graphics/VertexBuffer.h>
#include <Graphics/IndexBuffer.h>
#include <Graphics/Material.h>
#include <Graphics/Shader.h>
#include <Graphics/Texture.h>
#include <Maths/Matrix4.h>

#include <Resources/Resource.h>
#include <Resources/ResourceBank.h>

#include "MVPUniformUpdater.h"
#include "Texture0UniformUpdater.h"

#include <cstdio>
#ifdef GLES1
#include <Graphics/Renderer/GLES11/FixedFunctionGlVboRenderer.h>
#include <Graphics/State/GLES1/GLES1State.h>
#else
#include <Graphics/Renderer/GLES20/ShaderGLVboRenderer.h>
#include <Graphics/State/GLES2/GLES2State.h>
#endif

using namespace GLESGAE;

// Good 'ol Helper Functions
void controlCamera(Camera* const camera, Controller::KeyboardController* const keyboard);
Mesh* makeSprite(const Resource<Shader>& shader, const Resource<Texture>& texture);
Shader* makeSpriteShader();

ResourceBank<Matrix4> transformBank;
ResourceBank<VertexBuffer> vertexBank;
ResourceBank<Material> materialBank;
ResourceBank<IndexBuffer> indexBank;
ResourceBank<Texture> textureBank;
ResourceBank<Shader> shaderBank;
ResourceBank<Camera> cameraBank;
ResourceBank<Mesh> meshBank;

namespace LifecycleTest
{
	namespace Textures
	{
		Resources::Id MrSmileyFace = GLESGAE::INVALID;
	}

	namespace Shaders
	{
		Resources::Id SpriteShader = GLESGAE::INVALID;
	}

	namespace Cameras
	{
		Resources::Id SceneCamera = GLESGAE::INVALID;
	}

	namespace Meshes
	{
		Resources::Id MrSmileyFace = GLESGAE::INVALID;
	}

	namespace ShaderUniformUpdaters
	{
		Resources::Id MVPUpdater = GLESGAE::INVALID;
		Resources::Id TextureUpdater = GLESGAE::INVALID;
	}

	namespace IndexBuffers
	{
		Resources::Id SpriteIndexBuffer = GLESGAE::INVALID;
	}

	namespace VertexBuffers
	{
		Resources::Id SpriteVertexBuffer = GLESGAE::INVALID;
	}

	namespace Materials
	{
		Resources::Id SpriteMaterial = GLESGAE::INVALID;
	}

	namespace Transforms
	{
		Resources::Id SpriteTransform = GLESGAE::INVALID;
		Resources::Id SpriteTransform2 = GLESGAE::INVALID;
	}

	namespace Controllers
	{
		Controller::Id Keyboard = 0U;
	}

	namespace Graphics
	{
			RenderWindow* CurrentWindow = 0;
			RenderContext* CurrentContext = 0;
	}
}

TestLifecycle::TestLifecycle()
: Lifecycle()
, mScreenTarget(0)
, mHasResources(false)
{
}

void TestLifecycle::onCreate()
{
}

void TestLifecycle::onWindowCreate()
{
	Application* application(Application::getInstance());
	GraphicsSystem* graphicsSystem(application->getGraphicsSystem());
	EventSystem* eventSystem(application->getEventSystem());
	
	if (false == graphicsSystem->initialise()) { 
		//TODO: OH NOES! WE'VE DIEDED!
	}
	
	LifecycleTest::Graphics::CurrentWindow = graphicsSystem->getRenderPlatform()->createWindow("GLESGAE Application Lifecycle Test", 800, 480, 16, false);
	LifecycleTest::Graphics::CurrentWindow->open();
	eventSystem->bindToWindow(LifecycleTest::Graphics::CurrentWindow);
	
	LifecycleTest::Graphics::CurrentContext = graphicsSystem->getRenderPlatform()->createContext();
	LifecycleTest::Graphics::CurrentContext->bindToWindow(LifecycleTest::Graphics::CurrentWindow);
	LifecycleTest::Graphics::CurrentContext->initialise();
	mScreenTarget = LifecycleTest::Graphics::CurrentContext->createRenderTarget(RenderTarget::TARGET_SCREEN, RenderTarget::OPTIONS_WITH_COLOUR);
#ifdef GLES1
	FixedFunctionGlVboRenderer* renderer(new FixedFunctionGlVboRenderer);
#else
	ShaderGlVboRenderer* renderer(new ShaderGlVboRenderer);
#endif
	LifecycleTest::Graphics::CurrentContext->setRenderer(renderer);
	
	// Setup Render State settings
#ifdef GLES1
	GLES1State* currentState(reinterpret_cast<GLES1State*>(LifecycleTest::Graphics::CurrentContext->getRenderState()));
	currentState->setTexturingEnabled(true);
	currentState->setVertexPositionsEnabled(true);
	currentState->setAlphaBlendingEnabled(true);
#else
	GLES2State* currentState(reinterpret_cast<GLES2State*>(LifecycleTest::Graphics::CurrentContext->getRenderState()));
	currentState->setTexturingEnabled(true);
	currentState->setAlphaBlendingEnabled(true);
	currentState->addUniformUpdater(HashString("u_mvp"), new MVPUniformUpdater);
	currentState->addUniformUpdater(HashString("s_texture0"), new Texture0UniformUpdater);
	renderer->setState(currentState);
#endif
}

void TestLifecycle::onStart()
{
}

void TestLifecycle::onResume()
{
}

bool TestLifecycle::onLoop()
{
	if (0 == LifecycleTest::Graphics::CurrentWindow) // waiting on our window to become active.
		return true;
	
	if (false == mHasResources) {
		Application* application(Application::getInstance());
		InputSystem* inputSystem(application->getInputSystem());
		
		const Resource<Matrix4> transform(transformBank.add(new Matrix4));
		LifecycleTest::Transforms::SpriteTransform = transform.getId();
		
		const Resource<Matrix4> transform2(transformBank.add(new Matrix4));
		LifecycleTest::Transforms::SpriteTransform2 = transform2.getId();
		
		const Resource<Texture> textureId(textureBank.add(new Texture(new File("Texture.bmp"))));
		LifecycleTest::Textures::MrSmileyFace = textureId.getId();
		Texture* texture(textureBank.get(textureId));
		texture->load(Texture::FILTER_TRILINEAR);
		
		const Resource<Shader> shader(shaderBank.add(makeSpriteShader()));
		LifecycleTest::Shaders::SpriteShader = shader.getId();
		
		const Resource<Mesh> mesh(meshBank.add(makeSprite(shader, textureId)));
		LifecycleTest::Meshes::MrSmileyFace = mesh.getId();
		
		const Resource<Camera> cameraId(cameraBank.add(new Camera(Camera::CAMERA_3D)));
		LifecycleTest::Cameras::SceneCamera = cameraId.getId();
		Camera* camera(cameraBank.get(cameraId));
		camera->getTransformMatrix().setPosition(Vector3(0.0F, 0.0F, 5.0F));
		
		Controller::KeyboardController* myKeyboard(inputSystem->newKeyboard());
		LifecycleTest::Controllers::Keyboard = myKeyboard->getControllerId();
		
		mHasResources = true;
	}
	
	Application* application(Application::getInstance());
	EventSystem* eventSystem(application->getEventSystem());
	InputSystem* inputSystem(application->getInputSystem());
	RenderState* currentState(LifecycleTest::Graphics::CurrentContext->getRenderState());
	
	Camera* camera(cameraBank.get(LifecycleTest::Cameras::SceneCamera));
	Mesh* mesh(meshBank.get(LifecycleTest::Meshes::MrSmileyFace));
	Matrix4* transform(transformBank.get(LifecycleTest::Transforms::SpriteTransform));
	Matrix4* transform2(transformBank.get(LifecycleTest::Transforms::SpriteTransform2));
	Controller::KeyboardController* myKeyboard(inputSystem->getKeyboard(LifecycleTest::Controllers::Keyboard));
	
	Vector3 newPosition;
	transform->getPosition(&newPosition);
	newPosition.x() += 0.01F;
	newPosition.z() += 0.001F;
	transform->setPosition(newPosition);
	
	transform2->getPosition(&newPosition);
	newPosition.x() -= 0.01F;
	newPosition.z() += 0.001F;
	transform2->setPosition(newPosition);
	
	controlCamera(camera, myKeyboard);
	
	eventSystem->update();
	inputSystem->update();
	
	mScreenTarget->bind();
	currentState->setCamera(camera);
	LifecycleTest::Graphics::CurrentContext->drawMesh(mesh, transform);
	LifecycleTest::Graphics::CurrentContext->drawMesh(mesh, transform2);
	LifecycleTest::Graphics::CurrentContext->refresh();
	mScreenTarget->unbind();
	
	return !(myKeyboard->getKey(Controller::KEY_ESCAPE));
}

void TestLifecycle::onPause()
{
}

void TestLifecycle::onStop()
{
}

void TestLifecycle::onDestroy()
{
	mHasResources = false;
	LifecycleTest::Graphics::CurrentWindow = 0;
}

Mesh* makeSprite(const Resource<Shader>& shader, const Resource<Texture>& texture)
{
	float vertexData[20] = {// Position - 12 floats
					-1.0F, 1.0F, 0.0F,
					1.0F, 1.0F, 0.0F,
					1.0F, -1.0F, 0.0F,
					-1.0F, -1.0F, 0.0F,
					// Tex Coords - 8 floats
					0.0F, 1.0F,		// top left
					1.0F, 1.0F,		// top right
					1.0F, 0.0F,		// bottom right
					0.0F, 0.0F};	// bottom left
					

	unsigned int vertexSize = 20 * sizeof(float);

	unsigned short indexData[6] = { 0, 3, 2, 2, 1, 0 };
	unsigned int indexCount(6U);

	const Resource<VertexBuffer> newVertexBufferId(vertexBank.add(new VertexBuffer(reinterpret_cast<unsigned char*>(&vertexData), vertexSize)));
	VertexBuffer* newVertexBuffer(vertexBank.get(newVertexBufferId));
	newVertexBuffer->addFormatIdentifier(VertexBuffer::FORMAT_POSITION_3F, 4U);
	newVertexBuffer->addFormatIdentifier(VertexBuffer::FORMAT_TEXTURE_2F, 4U);
	
	const Resource<IndexBuffer> newIndexBufferId(indexBank.add(new IndexBuffer(reinterpret_cast<unsigned char*>(&indexData), indexCount, IndexBuffer::INDEX_UNSIGNED_SHORT, IndexBuffer::FORMAT_TRIANGLES)));
	IndexBuffer* newIndexBuffer(indexBank.get(newIndexBufferId));
	
	const Resource<Material> newMaterialId(materialBank.add(new Material));
	Material* newMaterial(materialBank.get(newMaterialId));
	newMaterial->setShader(shaderBank.get(shader));
	newMaterial->addTexture(textureBank.get(texture));

	return new Mesh(newVertexBuffer, newIndexBuffer, newMaterial);
}

void controlCamera(Camera* const camera, Controller::KeyboardController* const keyboard)
{
	Vector3 newPosition;
	camera->getTransformMatrix().getPosition(&newPosition);
	
	if (true == keyboard->getKey(Controller::KEY_ARROW_DOWN))
		newPosition.z() += 0.01F;
	
	if (true == keyboard->getKey(Controller::KEY_ARROW_UP))
		newPosition.z() -= 0.01F;
	
	if (true == keyboard->getKey(Controller::KEY_ARROW_LEFT))
		newPosition.x() -= 0.01F;
	
	if (true == keyboard->getKey(Controller::KEY_ARROW_RIGHT))
		newPosition.x() += 0.01F;
	
	camera->getTransformMatrix().setPosition(newPosition);
	camera->update();
}

#if defined(GLX)
	#include <Graphics/Context/Linux/GLee.h>
#endif

Shader* makeSpriteShader()
{
	std::string vShader =
		"attribute vec4 a_position;				\n"
		"attribute vec2 a_texCoord0;			\n"
		"varying vec2 v_texCoord0;				\n"
		"uniform mat4 u_mvp;					\n"
		"void main()							\n"
		"{										\n"
		"	gl_Position = u_mvp * a_position;	\n"
		"	v_texCoord0 = a_texCoord0;			\n"
		"}										\n";

	std::string fShader =
		#ifdef GLES2
		"precision mediump float;								\n"
		#endif
		"varying vec2 v_texCoord0;								\n"
		"uniform sampler2D s_texture0;							\n"
		"void main()											\n"
		"{														\n"
		"	gl_FragColor = texture2D(s_texture0, v_texCoord0);	\n"
		"	gl_FragColor.a = 1.0;								\n"
		"}														\n";

	#ifndef GLES1
		Shader* newShader(new Shader());
		newShader->createFromSource(vShader, fShader);

		return newShader;
	#else
		return 0;
	#endif
}
