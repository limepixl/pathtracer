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

	void append(T element)
	{
		// If the array has max elements, expand it
		if (size == internal_size)
		{
			// Increase old size by 1.5x (with some exceptions)
			unsigned long long old_size = internal_size;
			if (old_size <= 1)
				internal_size += 2;
			else
				internal_size += internal_size / 2;

			// Allocate new buffer with new size and
			// copy over the contents from the old buffer.
			T *tmp_data = new T[internal_size];
			memcpy(tmp_data, _data, old_size * sizeof(T));
			delete[] _data;
			_data = tmp_data;
		}

		_data[size] = element;
		size++;
	}

	void prepend(T element)
	{
		// If the array has max elements, expand it
		if (size == internal_size)
		{
			// Increase old size by 1.5x (with some exceptions)
			unsigned int old_size = internal_size;
			if (old_size <= 1)
				internal_size += 2;
			else
				internal_size += internal_size / 2;
		}

		// Shift all elements to the right
		T *tmp_data = new T[internal_size];
		if(size < internal_size)
			memcpy(tmp_data + 1, _data, size * sizeof(T));
		else
		{
			printf("ERROR!\n");
		}
		
		tmp_data[0] = element;

		delete[] _data;
		_data = tmp_data;
		size++;
	}

	T pop()
	{
		if (size == 0)
		{
			printf("ERROR: Popping from empty array!\n");
		}

		T result = _data[size - 1];
		_data[size - 1] = {};
		size--;

		return result;
	}

	T &back()
	{
		return _data[size - 1];
	}

	void clear()
	{
		if(size == 0)
			return;
		
		memset(_data, 0, size * sizeof(T));
		size = 0;
	}
};

// TODO: move to destructor + copy constructor
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
