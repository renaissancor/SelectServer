#pragma once

// RingBuffer.h 

class RingBuffer {
private:
	char* _buffer = nullptr;
	int _capacity = 0;
	int _head = 0;
	int _tail = 0;

private:
	void ResizeBuffer(const int newCapacity) noexcept;

public:
	inline bool IsEmpty() const noexcept { return _head == _tail; } 
	inline bool IsFull() const noexcept { return ((_tail + 1) % _capacity) == _head; } 

	inline int GetHeadIndex() const noexcept { return _head; }
	inline int GetTailIndex() const noexcept { return _tail; }

	inline int GetCapacity() const noexcept { return _capacity; }
	inline int GetUsedSize() const noexcept 
	{
		return (_head <= _tail) ? (_tail - _head) : (_capacity - _head + _tail); 
	}
	inline int GetFreeSize() const noexcept 
	{ return _capacity - GetUsedSize() - 1; }

	inline int DirectEnqueueSize() const noexcept
	{
		if (IsFull()) return 0;
		if (_tail >= _head)
			return (_capacity - _tail) - ((_head == 0) ? 1 : 0);
		else // _tail < _head
			return _head - _tail - 1;
	}

	inline int DirectDequeueSize() const noexcept
	{ return (_tail < _head) ? _capacity - _head : _tail - _head; }

	inline int MoveHead(int offset) noexcept {
		_head = (_head + offset) % _capacity;
		return offset;
	}

	inline int MoveTail(int offset) noexcept {
		_tail = (_tail + offset) % _capacity;
		return offset;
	}

	inline const uintptr_t GetBufferHeadPtr() const noexcept
		{ return reinterpret_cast<uintptr_t>(_buffer + _head); }

	inline const uintptr_t GetBufferTailPtr() const noexcept
		{ return reinterpret_cast<uintptr_t>(_buffer + _tail); }

	inline void ClearBuffer() noexcept {
		_head = 0;
		_tail = 0;
	}


	RingBuffer(int capacity = 8192);
	~RingBuffer();

	int Enqueue(const char* src, int size) noexcept; 
	int Dequeue(char* dst, int size) noexcept;
	int Peek(char* dst, int size) const noexcept; 


	int RecvFromTCP(SOCKET socket) noexcept; 
	int SendToTCP(SOCKET socket) noexcept; 

};
