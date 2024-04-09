#pragma once
#ifdef DEBUG
#define GL_CALL(func) func;GL_ERROR_FIND::findError()
#else
#define GL_CALL(func) func
#endif


#include <assert.h>
#include <string>
#include <iostream>

namespace GL_ERROR_FIND
{
	void findError();
}
