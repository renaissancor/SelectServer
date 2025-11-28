#pragma once

// RingBuffer.h 

class RingBuffer {
private:
	char* _buffer = nullptr;
	int _capacity = 0;
	int _head = 0;
	int _tail = 0;
	int _size = 0; 

private:
	void ResizeBuffer(const int newCapacity) noexcept; 

public:
	inline int GetCapacity() const noexcept { return _capacity; }
	inline int GetUsedSize() const noexcept { return _size; }
	inline int GetFreeSize() const noexcept { return _capacity - _size; } 

	inline int DirectEnqueueSize() const noexcept 
		{ return (_head < _tail) ? _capacity - _tail : _head - _tail; }

	inline int DirectDequeueSize() const noexcept 
		{ return (_tail < _head) ? _capacity - _head : _tail - _head; }

	inline int MoveHead(int offset) noexcept { 
		_head = (_head + offset) % _capacity; 
		_size -= offset; 
		return offset; 
	}

	inline int MoveTail(int offset) noexcept {
		_tail = (_tail + offset) % _capacity; 
		_size += offset; 
		return offset;
	}

	inline const uintptr_t GetBufferHeadPtr() const noexcept 
	{ return reinterpret_cast<uintptr_t>(_buffer + _head); }

	inline const uintptr_t GetBufferTailPtr() const noexcept 
	{ return reinterpret_cast<uintptr_t>(_buffer + _tail); }

	RingBuffer(int capacity = 4096);
	~RingBuffer();

	int Enqueue(const char* src, int size) noexcept;
	int Dequeue(char* dst, int size) noexcept;
	int Peek(char* dst, int size) const noexcept; // copy data without removing from buffer 
	
	void ClearBuffer() noexcept;

};
