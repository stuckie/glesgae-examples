#include <Platform/Platform.h>
#include <Platform/Lifecycle.h>

#include <Buffer/Buffer.h>
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
#include <Input/Controller.h>
#include <Maths/Matrix.h>
#include <Maths/Rect.h>
#include <States/StateStack.h>
#include <Time/Clock.h>
#include <Time/Timer.h>
#include <Utils/Array.h>
#include <Utils/Logger.h>
#include <Utils/Tiled/TiledJsonLoader.h>

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


GAE_Tiled_t* tilemap;
void loadTilemap(void) {
	/*unsigned int index = 0U;*/
	/*unsigned int size = 0U;*/
	
	GAE_File_t* mapFile = GAE_File_create("test.json", GAE_FILE_OPEN_READ, GAE_FILE_ASCII, 0);
	GAE_Buffer_t* buffer = GAE_Buffer_create(mapFile->fileSize);
	GAE_File_read(mapFile, buffer, mapFile->fileSize, 0);
	tilemap = GAE_TiledParser_create(buffer);
	GAE_Buffer_delete(buffer);
	GAE_File_delete(mapFile);
}

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
	GAE_PLATFORM->graphicsSystem->renderer->state->camera->right = 1.0F;
	GAE_PLATFORM->graphicsSystem->renderer->state->camera->bottom = 1.0F;
	GAE_PLATFORM->graphicsSystem->renderer->state->camera->top = 0.0F;
#endif

	GAE_PLATFORM->inputSystem = GAE_InputSystem_create(GAE_PLATFORM->eventSystem);
	GAE_InputSystem_newKeyboard(GAE_PLATFORM->inputSystem);
	GAE_InputSystem_newPointer(GAE_PLATFORM->inputSystem);

	GAE_PLATFORM->stateStack = GAE_StateStack_create();
	GAE_PLATFORM->mainClock = GAE_Clock_create();
	GAE_PLATFORM->logger = GAE_Logger_create();
}

void drawTilemap()
{
	unsigned int layerCount = GAE_Array_length(tilemap->layers);
	unsigned int index = 0U;

	for (index = 0U; index < layerCount; ++index)
		GAE_TiledParser_draw(tilemap, GAE_PLATFORM->graphicsSystem->renderer, index);
}

GAE_Sprite_t* sprite;
GAE_Texture_t* tex;
GAE_Sprite_t* createSprite(const char* texture)
{
	GAE_Sprite_t* s = GAE_Sprite_create(128, 128);

	GAE_File_t* file = GAE_File_create(texture, GAE_FILE_OPEN_READ, GAE_FILE_BINARY, 0);
	GAE_Buffer_t* data = GAE_Buffer_create(file->fileSize);
	GAE_File_read(file, data, GAE_FILE_READ_ALL, 0);
	GAE_File_delete(file);

	tex = GAE_Texture_create();
	GAE_Texture_load(tex, data, GAE_TEXTURE_FROM_FILE, GAE_TEXTURE_FROM_FILE);
	GAE_Buffer_delete(data);

	/* Sprite takes ownership of the frames */
	GAE_Sprite_addFrame(s, GAE_Frame_create(tex, 0, 0, 128, 128));
	GAE_Sprite_addFrame(s, GAE_Frame_create(tex, 128, 0, 128, 128));
	GAE_Sprite_addFrame(s, GAE_Frame_create(tex, 0, 128, 128, 128));
	GAE_Sprite_addFrame(s, GAE_Frame_create(tex, 128, 128, 128, 128));

	s->animSpeed = 1.0F;

	return s;
}

GAE_Timer_t* mainTimer;
void onStart()
{
    printf("onStart\n");
	sprite = createSprite("assets/Texture.bmp");
	mainTimer = GAE_Timer_create(GAE_PLATFORM->mainClock);
	loadTilemap();
}

void onResume()
{
	printf("onResume\n");
}

GAE_BOOL onLoop()
{
	GAE_Keyboard_t* keyboard = GAE_PLATFORM->inputSystem->keyboard;
	GAE_Clock_update(GAE_PLATFORM->mainClock);
	GAE_Timer_update(mainTimer, GAE_PLATFORM->mainClock);

	if (0 != GAE_PLATFORM->eventSystem)
		GAE_EventSystem_update(GAE_PLATFORM->eventSystem);
#if defined(GLX)
	GAE_Camera_update(GAE_PLATFORM->graphicsSystem->renderer->state->camera);
#endif
	drawTilemap();
	GAE_Sprite_update(sprite, mainTimer->deltaTime);
	GAE_GraphicsSystem_drawSprite(GAE_PLATFORM->graphicsSystem, sprite);
	GAE_RenderContext_update(GAE_PLATFORM->graphicsSystem->context);

    return !(keyboard->keys[GAE_KEY_ESCAPE]);
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
	GAE_InputSystem_removeKeyboard(GAE_PLATFORM->inputSystem, GAE_PLATFORM->inputSystem->keyboard);
	GAE_InputSystem_removePointer(GAE_PLATFORM->inputSystem, GAE_PLATFORM->inputSystem->pointer);
	GAE_InputSystem_delete(GAE_PLATFORM->inputSystem);
	GAE_EventSystem_delete(GAE_PLATFORM->eventSystem);
	GAE_Sprite_delete(sprite);
	GAE_Texture_delete(tex);
	GAE_GraphicsSystem_delete(GAE_PLATFORM->graphicsSystem);
	GAE_StateStack_delete(GAE_PLATFORM->stateStack);
	GAE_Timer_delete(mainTimer);
	GAE_Clock_delete(GAE_PLATFORM->mainClock);
	GAE_Logger_delete(GAE_PLATFORM->logger);

#if defined(SDL2)
	SDL_Quit();
#endif
}

