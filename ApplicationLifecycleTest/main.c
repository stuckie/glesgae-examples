#include <Platform/Platform.h>
#include <Platform/Lifecycle.h>

#include <Events/EventSystem.h>
#include <File/File.h>
#include <Graphics/Camera.h>
#include <Graphics/GraphicsSystem.h>
#include <Graphics/IndexBuffer.h>
#include <Graphics/Material.h>
#include <Graphics/Mesh.h>
#include <Graphics/Shader.h>
#include <Graphics/Sprite.h>
#include <Graphics/Texture.h>
#include <Graphics/VertexBuffer.h>
#include <Graphics/Context/RenderContext.h>
#include <Graphics/Renderer/Renderer.h>
#include <Graphics/State/RenderState.h>
#include <Graphics/Window/RenderWindow.h>
#include <Input/InputSystem.h>
#include <Maths/Matrix.h>
#include <States/StateStack.h>
#include <Time/Clock.h>
#include <Utils/Logger.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void onCreate();
void onWindowCreate();
void onStart();
void onResume();
GAE_BOOL onLoop();
void onPause();
void onStop();
void onDestroy();

void setLifecycle();

#if defined(GLX)
void MVPUpdate(const GLint uniformId, GAE_Camera_t* const camera, struct GAE_Material_s* const material, GAE_Matrix4_t* const transform) {
	GAE_Matrix4_t view;
	GAE_Matrix4_t projection;
	GAE_Matrix4_t mvp;
	
	GAE_UNUSED(material);

	GAE_Matrix4_copy(&view, &camera->view);
	GAE_Matrix4_copy(&projection, &camera->projection);
	GAE_Matrix4_copy(&mvp, transform);

	GAE_Matrix4_transpose(&mvp);
	GAE_Matrix4_mul(&mvp, &view);
	GAE_Matrix4_mul(&mvp, &projection);

	glUniformMatrix4fv(uniformId, 1U, GL_FALSE, mvp);
}

void Texture0Update(const GLint uniformId, struct GAE_Camera_s* const camera, struct GAE_Material_s* const material, GAE_Matrix4_t* const transform) {
	GAE_UNUSED(camera);
	GAE_UNUSED(material);
	GAE_UNUSED(transform);

	glUniform1i(uniformId, 0U);
}
#endif

PLATFORM_MAIN
	PLATFORM_INIT

	setLifecycle();

	PLATFORM_LOOP
PLATFORM_END

void setLifecycle()
{
	GAE_PLATFORM->lifecycle = (GAE_Lifecycle_t*)malloc(sizeof(GAE_Lifecycle_t));

	GAE_PLATFORM->lifecycle->onCreate = onCreate;
	GAE_PLATFORM->lifecycle->onWindowCreate = onWindowCreate;
	GAE_PLATFORM->lifecycle->onStart = onStart;
	GAE_PLATFORM->lifecycle->onResume = onResume;
	GAE_PLATFORM->lifecycle->onLoop = onLoop;
	GAE_PLATFORM->lifecycle->onPause = onPause;
	GAE_PLATFORM->lifecycle->onStop = onStop;
	GAE_PLATFORM->lifecycle->onDestroy = onDestroy;
}

void onCreate()
{
    printf("OnCreate\n");
}

#if defined(GLX)
	void GAE_X11_EventSystem_Bind_Window(GAE_EventSystem_t* commonSystem, GAE_RenderWindow_t* window);
#endif

void onWindowCreate()
{
#if defined(GLX)
	GAE_RenderState_t* state = 0;
#endif

    printf("onWindowCreate\n");
	
	GAE_PLATFORM->graphicsSystem = GAE_GraphicsSystem_create();
	GAE_PLATFORM->graphicsSystem->window = GAE_RenderWindow_create("GLESGAE", 480U, 320U, 16U, GAE_FALSE);
	GAE_RenderWindow_open(GAE_PLATFORM->graphicsSystem->window);

	GAE_PLATFORM->graphicsSystem->context = GAE_RenderContext_create();
	GAE_PLATFORM->graphicsSystem->context->window = GAE_PLATFORM->graphicsSystem->window;
	GAE_RenderContext_init(GAE_PLATFORM->graphicsSystem->context);

#if defined(SDL2)
	GAE_PLATFORM->graphicsSystem->renderer = GAE_SDL2Renderer_create(GAE_PLATFORM->graphicsSystem->window, -1, SDL_RENDERER_ACCELERATED);
	GAE_PLATFORM->graphicsSystem->context->renderer = GAE_PLATFORM->graphicsSystem->renderer;
#endif

	GAE_PLATFORM->eventSystem = GAE_EventSystem_create();

#if defined (GLX)
	GAE_PLATFORM->graphicsSystem->renderer = GAE_Renderer_create();
	GAE_X11_EventSystem_Bind_Window(GAE_PLATFORM->eventSystem, GAE_PLATFORM->graphicsSystem->window);

	state = GAE_PLATFORM->graphicsSystem->renderer->state;
	GAE_RenderState_setTexturingEnabled(state, GAE_TRUE);
	GAE_RenderState_setAlphaBlendingEnabled(state, GAE_TRUE);
	GAE_RenderState_addUniformUpdater(state, GAE_HashString_create("u_mvp"), MVPUpdate);
	GAE_RenderState_addUniformUpdater(state, GAE_HashString_create("s_texture0"), Texture0Update);
	
	GAE_PLATFORM->graphicsSystem->renderer->state->camera = GAE_Camera_create(GAE_CAMERA_TYPE_2D);
	GAE_PLATFORM->graphicsSystem->renderer->state->camera->left = 0.0F;
	GAE_PLATFORM->graphicsSystem->renderer->state->camera->right = 480.0F;
	GAE_PLATFORM->graphicsSystem->renderer->state->camera->bottom = 320.0F;
	GAE_PLATFORM->graphicsSystem->renderer->state->camera->top = 0.0F;
#endif

	GAE_PLATFORM->inputSystem = GAE_InputSystem_create(GAE_PLATFORM->eventSystem);
	GAE_InputSystem_newKeyboard(GAE_PLATFORM->inputSystem);
	GAE_InputSystem_newPointer(GAE_PLATFORM->inputSystem);

	GAE_PLATFORM->stateStack = GAE_StateStack_create();
	GAE_PLATFORM->mainClock = GAE_Clock_create();
	GAE_PLATFORM->logger = GAE_Logger_create();
}

GAE_Sprite_t* sprite;
void onStart()
{
    printf("onStart\n");
	sprite = GAE_Sprite_create("assets/Texture.bmp");
}

void onResume()
{
	printf("onResume\n");
}

int count = 0;
GAE_BOOL onLoop()
{
	if (0 != GAE_PLATFORM->eventSystem)
		GAE_EventSystem_update(GAE_PLATFORM->eventSystem);

	GAE_GraphicsSystem_drawSprite(GAE_PLATFORM->graphicsSystem, sprite);
	GAE_RenderContext_update(GAE_PLATFORM->graphicsSystem->context);

#if defined(SDL2)
	SDL_Delay(10);
#endif
    ++count;
    if (count > 120) return GAE_FALSE;
    return GAE_TRUE;
}

void onPause()
{
    printf("onPause\n");
}

void onStop()
{
    printf("onStop\n");
}

void onDestroy()
{
    printf("onDestroy\n");
}