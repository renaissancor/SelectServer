#include "stdafx.h"

#include "RingBuffer.h" 

RingBuffer::RingBuffer(int capacity)
	: _capacity(capacity), _head(0), _tail(0)
{
	if (_capacity < 4) _capacity = 4;
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
	Peek(reinterpret_cast<char*>(newBuffer), static_cast<int>(usedSize));
	delete[] _buffer;
	_buffer = newBuffer;
	_capacity = newCapacity;
	_head = 0;
	_tail = usedSize;
}

int RingBuffer::Enqueue(const char* src, int bytes) noexcept {
	if (bytes > GetFreeSize()) ResizeBuffer(_capacity * 2); // (_capacity + size);
	int firstChunk = min(bytes, DirectEnqueueSize());
	memcpy_s(_buffer + _tail, _capacity - _tail, src, firstChunk);
	int remaining = bytes - firstChunk;
	if (remaining == 0) {
		_tail = (_tail + firstChunk) % _capacity;
	}
	else {
		memcpy_s(_buffer, _capacity, src + firstChunk, remaining);
		_tail = remaining;
	}
	return bytes;
}

int RingBuffer::Dequeue(char* dst, int bytes) noexcept {
	if (bytes > GetUsedSize()) bytes = GetUsedSize();
	int firstChunk = min(bytes, _capacity - _head);
	memcpy_s(dst, bytes, _buffer + _head, firstChunk);
	int remaining = bytes - firstChunk;
	if (remaining == 0) {
		_head = (_head + firstChunk) % _capacity;
	}
	else {
		memcpy_s(dst + firstChunk, bytes - firstChunk, _buffer, remaining);
		_head = remaining;
	}
	return bytes;
}

int RingBuffer::Peek(char* dst, int bytes) const noexcept {
	if (bytes > GetUsedSize()) bytes = GetUsedSize();
	int firstChunk = min(bytes, _capacity - _head);
	memcpy_s(dst, bytes, _buffer + _head, firstChunk);
	int remaining = bytes - firstChunk;
	if (remaining > 0) {
		memcpy_s(dst + firstChunk, bytes - firstChunk, _buffer, remaining);
	}
	return bytes;
}


// Returns number of bytes recv, or negative error code on failure

int RingBuffer::RecvFromTCP(SOCKET socket) noexcept {
	int freeSize = GetFreeSize();
	
	if (freeSize == 0) {
		ResizeBuffer(_capacity * 2);
		freeSize = GetFreeSize();

		if (freeSize == 0)
			return -WSAEWOULDBLOCK;
	}
	
	int totalBytes = 0;

	int chunk1 = DirectEnqueueSize();
	if (chunk1 > freeSize) chunk1 = freeSize; 
	if (chunk1 <= 0) return totalBytes; 

	int received1 = ::recv(
		socket,
		_buffer + _tail,
		chunk1,
		0
	);

	fprintf_s(stdout, "[RecvFromTCP] Trying to recv up to %d bytes (chunk1=%d, freeSize=%d)\n",
		freeSize, chunk1, freeSize);

	if (received1 == 0) return 0;
	if (received1 == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK && totalBytes > 0)
			return totalBytes;
		return -err;
	}

	_tail = (_tail + received1) % _capacity;
	
	totalBytes += received1; 
	freeSize -= received1; 

	int chunk2 = DirectEnqueueSize(); 
	if (chunk2 > freeSize) chunk2 = freeSize; 
	if (chunk2 <= 0) return totalBytes; 

	int received2 = ::recv(
		socket,
		_buffer + _tail,
		chunk2,
		0
	);

	fprintf_s(stdout, "[RecvFromTCP] Trying to recv up to %d bytes (chunk2=%d, freeSize=%d)\n",
		freeSize, chunk2, freeSize);

	if (received2 == 0) return totalBytes > 0 ? totalBytes : 0; 

	if (received2 == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK && totalBytes > 0)
			return totalBytes;
		// return err;
		return -err; 
	}

	_tail = (_tail + received2) % _capacity;
	totalBytes += received2; 

	return totalBytes; 
}

// Returns number of bytes sent, or negative error code on failure
int RingBuffer::SendToTCP(SOCKET socket) noexcept {
	if (IsEmpty()) return 0; 
	int usedSize = GetUsedSize();
	if (usedSize == 0) return 0;

	int totalBytes = 0;

	int chunk1 = DirectDequeueSize();
	chunk1 = min(chunk1, usedSize);
	if (chunk1 > 0) {
		int sent1 = ::send(socket, _buffer + _head, chunk1, 0);
		
		if (sent1 == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK && totalBytes > 0)
				return totalBytes;
			return -err; 
		}
		_head = (_head + sent1) % _capacity;
		totalBytes += sent1;
		usedSize -= sent1;
	}

	int chunk2 = DirectDequeueSize();
	chunk2 = min(chunk2, usedSize);
	if (chunk2 > 0) {
		int sent2 = ::send(socket, _buffer + _head, chunk2, 0);
		
		if (sent2 == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK && totalBytes > 0)
				return totalBytes;
			return -err;
		}
		_head = (_head + sent2) % _capacity;
		totalBytes += sent2;
	}

	return totalBytes;
}
