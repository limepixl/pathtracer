#pragma once
#include "array.hpp"
#include <glad/glad.h>

template<class T>
void PushDataToSSBO(Array<T> &data, Array<GLuint> &ssbo_array)
{
	GLuint ssbo = 0;
	if (data.size > 0)
	{
		glCreateBuffers(1, &ssbo);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_array.size, ssbo);
		glNamedBufferStorage(ssbo, (GLsizeiptr) (data.size * sizeof(T)), &(data[0]), 0);
	}

	ssbo_array.append(ssbo);
}