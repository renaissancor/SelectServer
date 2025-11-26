#pragma once

// RingBuffer.h 

class RingBuffer {
private:
	char* _buffer = nullptr;
	int _capacity = 0;
	int _head = 0;
	int _tail = 0;
	int _size = 0;

public:
	inline int GetCapacity() const noexcept { return _capacity; }
	inline int GetUsedSize() const noexcept { return _size; }
	inline int GetFreeSize() const noexcept { return _capacity - _size; }

	RingBuffer(int capacity = 4096);
	~RingBuffer();

	int Enqueue(const char* src, int size) noexcept;
	int Dequeue(char* dst, int size) noexcept;
	void ClearBuffer() noexcept;

	// void Resize(); // Resize Buffer Capacity when needed during runtime
	int Peek(char* dst, int size) const noexcept; // copy data without removing from buffer 
};
