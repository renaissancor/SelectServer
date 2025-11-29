#include "stdafx.h"

#include "RingBuffer.h" 

RingBuffer::RingBuffer(size_t capacity)
	: _capacity(capacity), _head(0), _tail(0), _size(0)
{
	_buffer = new uint8_t[_capacity];
}

RingBuffer::~RingBuffer()
{
	delete[] _buffer;
}

void RingBuffer::ResizeBuffer(const size_t newCapacity) noexcept {
	if (newCapacity <= _capacity) return;
	uint8_t* newBuffer = new uint8_t[newCapacity];
	size_t usedSize = GetUsedSize();
	Peek(reinterpret_cast<char*>(newBuffer), static_cast<int>(usedSize));
	delete[] _buffer;
	_buffer = newBuffer;
	_capacity = newCapacity;
	_head = 0;
	_tail = usedSize;
}


void RingBuffer::ClearBuffer() noexcept {
	_head = 0;
	_tail = 0;
	_size = 0;
}

size_t RingBuffer::Enqueue(const char* src, size_t size) noexcept {
	if (size > GetFreeSize()) ResizeBuffer(_capacity * 2); // (_capacity + size);
	size_t firstChunk = min(size, _capacity - _tail);
	memcpy_s(_buffer + _tail, _capacity - _tail, src, firstChunk);
	size_t remaining = size - firstChunk;
	if (remaining == 0) {
		_tail = (_tail + firstChunk) % _capacity;
	}
	else {
		memcpy_s(_buffer, _capacity, src + firstChunk, remaining);
		_tail = remaining;
	}
	_size += size;
	return size;
}

size_t RingBuffer::Dequeue(char* dst, size_t size) noexcept {
	if (size > GetUsedSize()) size = GetUsedSize();
	size_t firstChunk = min(size, _capacity - _head);
	memcpy_s(dst, size, _buffer + _head, firstChunk);
	size_t remaining = size - firstChunk;
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

size_t RingBuffer::Peek(char* dst, size_t size) const noexcept {
	if (size > GetUsedSize()) size = GetUsedSize();
	size_t firstChunk = min(size, _capacity - _head);
	memcpy_s(dst, size, _buffer + _head, firstChunk);
	size_t remaining = size - firstChunk;
	if (remaining > 0) {
		memcpy_s(dst + firstChunk, size - firstChunk, _buffer, remaining);
	}
	return size;
}

