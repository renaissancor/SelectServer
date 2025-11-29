#pragma once

// RingBuffer.h 

class RingBuffer {
private:
	uint8_t* _buffer = nullptr;
	size_t _capacity = 0;
	size_t _head = 0;
	size_t _tail = 0;
	size_t _size = 0;

private:
	void ResizeBuffer(const size_t newCapacity) noexcept;

public:
	inline size_t GetCapacity() const noexcept { return _capacity; }
	inline size_t GetUsedSize() const noexcept { return _size; }
	inline size_t GetFreeSize() const noexcept { return _capacity - _size; }

	inline size_t DirectEnqueueSize() const noexcept
	{ return (_head < _tail) ? _capacity - _tail : _head - _tail; }

	inline size_t DirectDequeueSize() const noexcept
	{ return (_tail < _head) ? _capacity - _head : _tail - _head; }

	inline size_t MoveHead(size_t offset) noexcept {
		_head = (_head + offset) % _capacity;
		_size -= offset;
		return offset;
	}

	inline size_t MoveTail(size_t offset) noexcept {
		_tail = (_tail + offset) % _capacity;
		_size += offset;
		return offset;
	}

	inline const uintptr_t GetBufferHeadPtr() const noexcept
		{ return reinterpret_cast<uintptr_t>(_buffer + _head); }

	inline const uintptr_t GetBufferTailPtr() const noexcept
		{ return reinterpret_cast<uintptr_t>(_buffer + _tail); }

	RingBuffer(size_t capacity = 4096);
	~RingBuffer();

	size_t Enqueue(const char* src, size_t size) noexcept; 
	size_t Dequeue(char* dst, size_t size) noexcept;
	size_t Peek(char* dst, size_t size) const noexcept; 

	void ClearBuffer() noexcept;

};
