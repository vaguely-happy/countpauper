#include "stdafx.h"
#include <gl/GLew.h>
#include "Error.h"

namespace Engine
{
    void CheckGLError()
    {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
            throw std::runtime_error("OpenGL error");
    }

}