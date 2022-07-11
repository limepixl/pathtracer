#pragma once
#include <cstdio>
#include <cstring>

#define ARRAY_STARTING_SIZE 0

template <typename T>
struct Array
{
	T *_data;
	unsigned long long size;
	unsigned long long internal_size; // the size of the allocated memory

	T &operator[](unsigned int i)
	{
		if(i >= size) 
		{ 
			printf("ERROR!\n"); 
		}
		return _data[i];
	}

	Array(unsigned long long count, T *data = nullptr)
	{
		internal_size = count;

		if (data == nullptr)
		{
			this->size = 0;
			this->_data = new T[count];
		}
		else
		{
			this->size = count;
			this->_data = data;
		}
	}

	Array()
	{
		size = ARRAY_STARTING_SIZE;
		internal_size = ARRAY_STARTING_SIZE;
		_data = nullptr;
		if (size > 0)
		{
			_data = new T[size];
		}
	}
};

template <typename T>
void AppendToArray(Array<T> &arr, T element)
{
	// If the array has max elements, expand it
	if (arr.size == arr.internal_size)
	{
		// Increase old size by 1.5x (with some exceptions)
		unsigned long long old_size = arr.internal_size;
		if (old_size <= 1)
			arr.internal_size += 2;
		else
			arr.internal_size += arr.internal_size / 2;

		// Allocate new buffer with new size and
		// copy over the contents from the old buffer.
		T *tmp_data = new T[arr.internal_size];
		memcpy(tmp_data, arr._data, old_size * sizeof(T));
		delete[] arr._data;
		arr._data = tmp_data;
	}

	arr._data[arr.size] = element;
	arr.size++;
}

template <typename T>
void PrependToArray(Array<T> &arr, T &element)
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
	}

	// Shift all elements to the right
	T *tmp_data = new T[arr.internal_size];
	if(arr.size < arr.internal_size)
		memcpy(tmp_data + 1, arr._data, arr.size * sizeof(T));
	else
	{
		printf("ERROR 123!\n");
	}
	
	tmp_data[0] = element;

	delete[] arr._data;
	arr._data = tmp_data;
	arr.size++;
}

template <typename T>
T PopFromArray(Array<T> &arr)
{
	// TODO: this is an error and should be handled
	if (arr.size == 0)
	{
		printf("ERROR: Popping from empty array!\n");
	}

	T result = arr._data[arr.size - 1];
	arr._data[arr.size - 1] = {};
	arr.size--;

	return result;
}

template <typename T>
T &Back(Array<T> &arr)
{
	return arr._data[arr.size - 1];
}

template <typename T>
void ClearArray(Array<T> &arr)
{
	if (arr.size == 0)
		return;

	memset(arr._data, 0, arr.size * sizeof(T));
	arr.size = 0;
}

template <typename T>
void DeallocateArray(Array<T> &arr)
{
	if (arr._data != nullptr)
	{
		delete[] arr._data;
		arr._data = nullptr;
	}

	arr.size = 0;
	arr.internal_size = 0;
}
