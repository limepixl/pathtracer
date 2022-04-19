#pragma once
#include "../scene/sphere.hpp"
#include "../scene/triangle.hpp"
#include "../defines.hpp"

#define ARRAY_STARTING_SIZE 0

template <typename T>
struct Array
{
	T *data;
	unsigned int size;
	unsigned int internal_size; // the size of the allocated memory

	T &operator[](unsigned int i)
	{
		return data[i];
	}
};

template <typename T>
Array<T> CreateArray(unsigned int size, T *data = nullptr)
{
	Array<T> res {};
	res.internal_size = size;

	if (data == nullptr)
	{
		res.size = 0;
		res.data = new T[size];
	}
	else
	{
		res.size = size;
		res.data = data;
	}

	return res;
}

template <typename T>
Array<T> CreateArray()
{
	Array<T> res {};
	res.size = ARRAY_STARTING_SIZE;
	res.internal_size = ARRAY_STARTING_SIZE;
	res.data = nullptr;
	if(res.size > 0)
	{
		res.data = new T[res.size];
	}

	return res;
}

template <typename T>
void AppendToArray(Array<T> &arr, T &element)
{
	// If the array has max elements, expand it
	if(arr.size == arr.internal_size)
	{
		// Increase old size by 1.5x
		unsigned int old_size = arr.internal_size;
		arr.internal_size += arr.internal_size / 2;

		// Allocate new buffer with new size and 
		// copy over the contents from the old buffer.
		T *tmp_data = new T[arr.internal_size];
		memcpy(tmp_data, arr.data, old_size * sizeof(T));
		delete[] arr.data;
		arr.data = tmp_data;
	}

	arr.data[arr.size] = element;
	arr.size++;
}

template <typename T>
void ClearArray(Array<T> &arr)
{
	if(arr.size == 0) return;

	memset(arr.data, 0, arr.size * sizeof(T));
	arr.size = 0;
}

template <typename T>
void DeallocateArray(Array<T> &arr)
{
	if (arr.data != nullptr)
	{
		delete[] arr.data;
		arr.data = nullptr;
	}

	arr.size = 0;
	arr.internal_size = 0;
}