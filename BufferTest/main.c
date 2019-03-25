#include <Platform/Platform.h>
#include <Platform/Lifecycle.h>

#include <Buffer/Buffer.h>
#include <File/File.h>
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

typedef struct GAE_STRING_s
{
	unsigned int length;
	char* string;
} GAE_STRING;

typedef struct BufferTest_s
{
	GAE_BOOL bBool;
	GAE_BYTE bNumber;
	short sNumber;
	int iNumber;
	long lNumber;
	float fNumber;
	double dNumber;
	char cLetter;
} BufferTest_t;

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
}

void onCreate()
{
	GAE_Buffer_t* buffer = GAE_Buffer_create(5 * sizeof(uint32_t));
	GAE_File_t* file = GAE_File_create("test.out", GAE_FILE_OPEN_WRITE, GAE_FILE_BINARY, NULL);
	uint32_t i = 0;
	unsigned int j = 0;
	for (i = 0; i < 5; ++i)
		GAE_Buffer_write_uint32(buffer, &i);
	
	GAE_File_write(file, buffer, NULL);
	GAE_File_delete(file);
	GAE_Buffer_delete(buffer);
	
	file = GAE_File_create("test.out", GAE_FILE_OPEN_READ, GAE_FILE_BINARY, NULL);
	buffer = GAE_Buffer_create(5 * sizeof(uint32_t));
	GAE_File_read(file, buffer, 5 * sizeof(uint32_t), NULL);
	GAE_Buffer_seek(buffer, 0);

	for (i = 0; i < 5; ++i) {
		GAE_Buffer_read_uint32(buffer, &j);
		printf("%d\n", j);
	}
	
    printf("OnCreate\n");
}

void onWindowCreate()
{
	GAE_BYTE* memory = malloc(sizeof(unsigned int));
	unsigned int number = 42;
	memcpy(memory, &number, sizeof(unsigned int));
	printf("%d\t%d\n", (unsigned int)*memory, number);
	
    printf("onWindowCreate\n");

	GAE_PLATFORM->logger = GAE_Logger_create();
}

void onStart()
{
	BufferTest_t bufferOut;
	BufferTest_t bufferIn;
	GAE_Buffer_t* buffer = GAE_Buffer_create(sizeof(BufferTest_t));
	GAE_File_t* file = GAE_File_create("test.out", GAE_FILE_OPEN_WRITE, GAE_FILE_BINARY, NULL);

	bufferOut.bBool = GAE_TRUE;
	bufferOut.bNumber = 128;
	bufferOut.sNumber = 16553;
	bufferOut.iNumber = 32312;
	bufferOut.lNumber = 64231;
	bufferOut.fNumber = 3.15F;
	bufferOut.dNumber = 0.0024;
	bufferOut.cLetter = 'a';
	
	GAE_Buffer_write(buffer, (GAE_BYTE*)&bufferOut, sizeof(BufferTest_t));
	GAE_File_write(file, buffer, NULL);
	GAE_File_delete(file);
	GAE_Buffer_delete(buffer);
	
	file = GAE_File_create("test.out", GAE_FILE_OPEN_READ, GAE_FILE_BINARY, NULL);
	buffer = GAE_Buffer_create(sizeof(BufferTest_t));
	GAE_File_read(file, buffer, sizeof(BufferTest_t), NULL);
	GAE_Buffer_seek(buffer, 0);

	GAE_Buffer_read(buffer, (GAE_BYTE*)&bufferIn, sizeof(BufferTest_t));

	printf("Out\t\tIn\n");
	printf("%d\t\t%d\n", bufferOut.bBool, bufferIn.bBool);
	printf("%d\t\t%d\n", bufferOut.bNumber, bufferIn.bNumber);
	printf("%d\t\t%d\n", bufferOut.sNumber, bufferIn.sNumber);
	printf("%d\t\t%d\n", bufferOut.iNumber, bufferIn.iNumber);
	printf("%ld\t\t%ld\n", bufferOut.lNumber, bufferIn.lNumber);
	printf("%f\t%f\n", bufferOut.fNumber, bufferIn.fNumber);
	printf("%f\t%f\n", bufferOut.dNumber, bufferIn.dNumber);
	printf("%c\t\t%c\n", bufferOut.cLetter, bufferIn.cLetter);
	
	GAE_Buffer_delete(buffer);
	GAE_File_delete(file);
	
    printf("onStart\n");
}

void onResume()
{
	printf("onResume\n");
}

GAE_BOOL onLoop()
{
	/* Just quit.. nothing else to really do */
    return GAE_FALSE;
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
	GAE_Logger_delete(GAE_PLATFORM->logger);

#if defined(SDL2)
	SDL_Quit();
#endif
}

