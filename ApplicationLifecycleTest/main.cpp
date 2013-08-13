#include <Platform/Application.h>
#include <Platform/Platform.h>
#include "TestLifecycle.h"

using namespace GLESGAE;

PLATFORM_MAIN
	Application* application(Application::getInstance());
	application->setLifecycle(new TestLifecycle);
	
	Platform* platform(new Platform);
	PLATFORM_INIT
	application->setPlatform(platform);
	
	PLATFORM_LOOP
END_MAIN
