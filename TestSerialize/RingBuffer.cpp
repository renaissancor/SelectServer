#include "stdafx.h"
#include "RingBuffer.h" 

RingBuffer::RingBuffer(size_t capacity) noexcept :
	_capacity(capacity), _head(0), _tail(0), _buffer(nullptr)
{
	if (_capacity < 32) _capacity = 32;
	_buffer = new char[_capacity];
}

RingBuffer::~RingBuffer() {
	if (_buffer) delete[] _buffer;
	_buffer = nullptr;
}

void RingBuffer::ResizeBuffer(const size_t newCapacity) noexcept {
	if (newCapacity <= _capacity) return;
	char* newBuffer = new char[newCapacity];
	size_t usedSize = GetUsedSize();
	Peek(reinterpret_cast<char*>(newBuffer), static_cast<size_t>(usedSize));
	delete[] _buffer;
	_buffer = newBuffer;
	_capacity = newCapacity;
	_head = 0;
	_tail = usedSize;
}

size_t RingBuffer::Peek(char* dst, size_t bytes) const noexcept {
	if (bytes > GetUsedSize()) bytes = GetUsedSize();
	size_t firstChunk = min(bytes, _capacity - _head);
	memcpy_s(dst, bytes, _buffer + _head, firstChunk);
	size_t remaining = bytes - firstChunk;
	if (remaining > 0) {
		memcpy_s(dst + firstChunk, bytes - firstChunk, _buffer, remaining);
	}
	return bytes;
}

size_t RingBuffer::Enqueue(const char* src, size_t size) noexcept {
	if (size > GetFreeSize()) ResizeBuffer(_capacity * 2); // (_capacity + size);
	size_t firstChunk = min(size, DirectEnqueueSize());
	memcpy_s(_buffer + _tail, _capacity - _tail, src, firstChunk);
	size_t remaining = size - firstChunk;
	if (remaining == 0) {
		_tail = (_tail + firstChunk) % _capacity;
	}
	else {
		memcpy_s(_buffer, _capacity, src + firstChunk, remaining);
		_tail = remaining;
	}
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
	return size;
}

