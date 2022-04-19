#pragma once
#include <cstring>

#define ARRAY_STARTING_SIZE 0

template <typename T>
struct Array
{
	T *data;
	unsigned long size;
	unsigned long internal_size; // the size of the allocated memory

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
	if (res.size > 0)
	{
		res.data = new T[res.size];
	}

	return res;
}

template <typename T>
void AppendToArray(Array<T> &arr, T &element)
{
	// If the array has max elements, expand it
	if (arr.size == arr.internal_size)
	{
		// Increase old size by 1.5x (with some exceptions)
		unsigned int old_size = arr.internal_size;
		if (old_size <= 1)
			arr.internal_size += 2;
		else
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

#include <cstdio>

template <typename T>
T PopFromArray(Array<T> &arr)
{
	// TODO: this is an error and should be handled
	if (arr.size == 0)
	{
		printf("ERROR: Popping from empty array!\n");
		return { 0 };
	}

	T result = arr.data[arr.size - 1];
	arr.data[arr.size - 1] = { 0 };
	arr.size--;

	return result;
}

template <typename T>
void ClearArray(Array<T> &arr)
{
	if (arr.size == 0)
		return;

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