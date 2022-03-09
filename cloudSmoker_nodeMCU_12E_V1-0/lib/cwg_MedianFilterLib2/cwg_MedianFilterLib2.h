/***************************************************
Copyright (c) 2017 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/

#ifndef _MedianFilter2Lib_h
#define _MedianFilter2Lib_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

template <typename T>
class MedianFilter2
{

typedef T (MedianFilter2::*Action)(T);

public:
	MedianFilter2<T>(const size_t windowSize);

    ~MedianFilter2<T>(); // prototype the destructor


	T AddValue(T item);
	T GetFiltered() const;

private:
	struct node
	{
		struct node *next;      // Point to the next element in order (Apunta al siguiente elemento en orden)
		T value;
	};

	Action addValue;			// Pointer to allow switching between algorithm 3 or N (Puntero para permitir cambiar entra algoritmo 3 o N)
	T addValue3(T item);
	T addValueN(T item);
	T median3(T a, T b, T c);

	int _windowSize;
	T _lastFiltered;

	const static int _stopper = 0;
	struct node *_buffer;		// Buffer for items in order of arrival (Buffer para los elementos por orden de llegada
	struct node *_iterator; 

	struct node _smaller;
	struct node _bigger;
};


template<typename T>
MedianFilter2<T>::MedianFilter2(const size_t windowSize)
{
	_windowSize = windowSize;
	_buffer = new node[windowSize];
	_iterator = _buffer;

	_smaller = { nullptr, _stopper };
	_bigger = { &_smaller, 0 };

	if(_windowSize == 3)
		addValue = &MedianFilter2::addValue3;
	else
		addValue = &MedianFilter2::addValueN;
}

// implement the desructor
template<typename T>
MedianFilter2<T>::~MedianFilter2() {
	delete [] _buffer;
}



template<typename T>
T MedianFilter2<T>::AddValue(T value)
{
	return (*this.*addValue)(value);
}

template<typename T>
T MedianFilter2<T>::GetFiltered() const
{
	return _lastFiltered;
}

template<typename T>
T MedianFilter2<T>::addValueN(T value)
{
	struct node *_successor;
	struct node *_accessor; 
	struct node *_accessorPrev;
	struct node *_median;

	if (value == _stopper)
		++value;

	if ((++_iterator - _buffer) >= _windowSize)
		_iterator = _buffer;

	_iterator->value = value;
	_successor = _iterator->next;
	_median = &_bigger;
	_accessor = &_bigger;
	_accessorPrev = nullptr;

	if (_accessor->next == _iterator)
		_accessor->next = _successor;

	_accessorPrev = _accessor; 
	_accessor = _accessor->next;

	for (int iCount = 0; iCount < _windowSize; ++iCount)
	{
		// Management of odd elements (Gestion de elementos impares)
		if (_accessor->next == _iterator)
			_accessor->next = _successor;

		if (_accessor->value < value)
		{
			_iterator->next = _accessorPrev->next;
			_accessorPrev->next = _iterator;
			value = _stopper;
		};

		_median = _median->next;
		if (_accessor == &_smaller)
			break;

		_accessorPrev = _accessor;  
		_accessor = _accessor->next;

		// Management of even elements (Gestion de elementos pares)
		if (_accessor->next == _iterator)
			_accessor->next = _successor;

		if (_accessor->value < value)
		{
			_iterator->next = _accessorPrev->next;
			_accessorPrev->next = _iterator;
			value = _stopper;
		}

		if (_accessor == &_smaller)
			break;

		_accessorPrev = _accessor;
		_accessor = _accessor->next;
	}

	_lastFiltered = _median->value;
	return _lastFiltered;
}

template<typename T>
T MedianFilter2<T>::addValue3(T value)
{
	if ((++_iterator - _buffer) >= 3)
		_iterator = _buffer;
	_iterator->value = value;

	return median3(_buffer->value, (_buffer + 1)->value, (_buffer + 2)->value);
}
	
template<typename T>
T MedianFilter2<T>::median3(T a, T b, T c)
{
	if ((a <= b) && (a <= c))
		return (b <= c) ? b : c;
	if ((b <= a) && (b <= c))
		return (a <= c) ? a : c;
	return (a <= b) ? a : b;
}

#endif

