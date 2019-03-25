#include <Platform/Platform.h>
#include <Platform/Lifecycle.h>

#include <Audio/AudioSystem.h>
#include <Audio/Sound.h>
#include <Buffer/Buffer.h>
#include <Events/EventSystem.h>
#include <File/File.h>
#include <Graphics/GraphicsSystem.h>
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

typedef struct AudioData_s
{
	GAE_Sound_t* ping;
	GAE_Sound_t* music;
} AudioData_t;

PLATFORM_MAIN
	PLATFORM_INIT

	setLifecycle();

	PLATFORM_LOOP
PLATFORM_END

void setLifecycle()
{
	GAE_PLATFORM->lifecycle = malloc(sizeof(GAE_Lifecycle_t));

	GAE_PLATFORM->lifecycle->onCreate = onCreate;
	GAE_PLATFORM->lifecycle->onWindowCreate = onWindowCreate;
	GAE_PLATFORM->lifecycle->onStart = onStart;
	GAE_PLATFORM->lifecycle->onResume = onResume;
	GAE_PLATFORM->lifecycle->onLoop = onLoop;
	GAE_PLATFORM->lifecycle->onPause = onPause;
	GAE_PLATFORM->lifecycle->onStop = onStop;
	GAE_PLATFORM->lifecycle->onDestroy = onDestroy;
	
	GAE_PLATFORM->userData = malloc(sizeof(AudioData_t));
}

void onCreate()
{
    printf("OnCreate\n");
}

void onWindowCreate()
{
    printf("onWindowCreate\n");
	
	GAE_PLATFORM->audioSystem = GAE_AudioSystem_create(48000, 2, 512, GAE_AudioFormat_s16);
	
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

	GAE_PLATFORM->inputSystem = GAE_InputSystem_create(GAE_PLATFORM->eventSystem);
	GAE_InputSystem_newKeyboard(GAE_PLATFORM->inputSystem);
	GAE_InputSystem_newPointer(GAE_PLATFORM->inputSystem);

	GAE_PLATFORM->stateStack = GAE_StateStack_create();
	GAE_PLATFORM->mainClock = GAE_Clock_create();
	GAE_PLATFORM->logger = GAE_Logger_create();
}

void onStart()
{
	GAE_Sound_t* sound = GAE_Sound_createFrom("assets/ping.wav");
	GAE_Sound_t* music = GAE_Sound_createFrom("assets/SplooshIngame.ogg");
	AudioData_t* audioData = GAE_PLATFORM->userData;
	
	audioData->ping = sound;
	audioData->music = music;
	
	GAE_Sound_setLooping(music, GAE_TRUE);
	GAE_Sound_play(music);
	
    printf("onStart\n");
}

void onResume()
{
	printf("onResume\n");
}

GAE_BOOL onLoop()
{
	AudioData_t* audioData = GAE_PLATFORM->userData;
	GAE_Keyboard_t* keyboard = GAE_PLATFORM->inputSystem->keyboard;
	GAE_Clock_update(GAE_PLATFORM->mainClock);

	if (0 != GAE_PLATFORM->eventSystem)
		GAE_EventSystem_update(GAE_PLATFORM->eventSystem);

	GAE_RenderContext_update(GAE_PLATFORM->graphicsSystem->context);
	GAE_Sound_update(audioData->music);
	
	if (keyboard->keys[GAE_KEY_P])
		GAE_Sound_play(audioData->ping);

#if defined(SDL2)		
	SDL_Delay(32); /* lets not eat up all the time */
#endif

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
	GAE_GraphicsSystem_delete(GAE_PLATFORM->graphicsSystem);
	GAE_StateStack_delete(GAE_PLATFORM->stateStack);
	GAE_Clock_delete(GAE_PLATFORM->mainClock);
	GAE_Logger_delete(GAE_PLATFORM->logger);

#if defined(SDL2)
	SDL_Quit();
#endif
}

