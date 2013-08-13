#include "Texture0UniformUpdater.h"

#if defined(GLX)
	#include <Graphics/Context/Linux/GLee.h>
#elif defined(GLES2)
	#if defined(PANDORA) || defined(ANDROID)
		#include <GLES2/gl2.h>
	#endif
#endif

using namespace GLESGAE;

void Texture0UniformUpdater::update(const GLint uniformId, Camera* const, Material* const, Matrix4* const)
{
#ifndef GLES1
	glUniform1i(uniformId, 0U);
#endif
}

