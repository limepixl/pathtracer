#pragma once
#include <cstdio>
#include <cstring>

#define ARRAY_STARTING_SIZE 0

template <typename T>
struct Array
{
	T *_data;
	unsigned int size;
	unsigned int internal_size; // the size of the allocated memory

	T &operator[](unsigned int i)
	{
		if(i >= size) 
		{ 
			printf("ERROR!\n"); 
		}
		return _data[i];
	}

	explicit Array(unsigned int count, T *data = nullptr)
	{
		internal_size = count;

		if (data == nullptr)
		{
			size = 0;
			_data = new T[count];
		}
		else
		{
			size = count;
			_data = data;
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

	Array(const Array &other)
	{
		size = other.size;
		internal_size = size;
		_data = nullptr;
		if(size > 0)
		{
			_data = new T[size];
			memcpy(_data, other._data, size * sizeof(T));
		}
	}

	Array& operator=(const Array &other)
	{
		if(size > 0)
		{
			delete[] _data;
			_data = nullptr;
		}

		if(other.internal_size > 0)
		{
			_data = new T[other.internal_size];
			if(other.size > 0)
				memcpy(_data, other._data, other.size * sizeof(T));
		}
		size = other.size;
		internal_size = other.internal_size;
		return *this;
	}

	~Array()
	{
		if(size > 0)
		{
			delete[] _data;
			_data = nullptr;
		}
	}

	void append(T element)
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
