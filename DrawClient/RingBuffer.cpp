#include "stdafx.h"
#include "RingBuffer.h"

// RingBuffer.cpp 

RingBuffer::RingBuffer(int capacity = 8192) : 
	_capacity(capacity), _head(0), _tail(0)
{
	_buffer = new char[_capacity];
}

RingBuffer::~RingBuffer() 
{
	delete[] _buffer;
}

void RingBuffer::ResizeBuffer(const int newCapacity) noexcept {
	if (newCapacity <= _capacity) return;
	char* newBuffer = new char[newCapacity];
	int usedSize = GetUsedSize();
	Peek(newBuffer, usedSize);
	delete[] _buffer;
	_buffer = newBuffer;
	_capacity = newCapacity;
	_head = 0;
	_tail = usedSize;
	fwprintf_s(stdout, L"RingBuffer resized to %d bytes.\n", newCapacity); 
}

int RingBuffer::Enqueue(const char* src, int size) noexcept {
	if (size > GetFreeSize()) ResizeBuffer(_capacity + size); 
	int firstChunk = min(size, _capacity - _tail);
	memcpy_s(_buffer + _tail, _capacity - _tail, src, firstChunk);
	int remaining = size - firstChunk;
	if (remaining == 0) {
		// All data fits in the first chunk 
		_tail = (_tail + firstChunk) % _capacity;
	}
	else {
		// data size is larger than the first chunk 
		memcpy_s(_buffer, _capacity, src + firstChunk, remaining);
		_tail = remaining;
	}
	_size += size;
	return size;
}

int RingBuffer::Dequeue(char* dst, int size) noexcept {
	if (size > GetUsedSize()) size = GetUsedSize();

	int firstChunk = min(size, _capacity - _head);
	memcpy_s(dst, size, _buffer + _head, firstChunk);
	int remaining = size - firstChunk;
	if (remaining == 0) {
		_head = (_head + firstChunk) % _capacity;
	}
	else {
		memcpy_s(dst + firstChunk, size - firstChunk, _buffer, remaining);
		_head = remaining;
	}
	_size -= size;
	return size;
}

void RingBuffer::ClearBuffer() noexcept {
	memset(_buffer, 0, _capacity);
	_head = 0;
	_tail = 0;
	_size = 0;
}

int RingBuffer::Peek(char* dst, int size) const noexcept {
	if (size > GetUsedSize()) size = GetUsedSize();

	int firstChunk = min(size, _capacity - _head);
	memcpy_s(dst, size, _buffer + _head, firstChunk);
	int remaining = size - firstChunk;
	if (remaining > 0) {
		memcpy_s(dst + firstChunk, size - firstChunk, _buffer, remaining);
	}

	return size;
}
