#pragma once

// RingBuffer.h 

class RingBuffer {
protected:
	char* _buffer = nullptr;
	size_t _capacity = 0;
	size_t _head = 0;
	size_t _tail = 0;

public:
	inline size_t GetHeadIndex() const noexcept { return _head; }
	inline size_t GetTailIndex() const noexcept { return _tail; }
	inline size_t GetCapacity() const noexcept { return _capacity; }

	inline size_t DirectEnqueueSize() const noexcept
	{
		if (IsFull()) return 0;
		if (_tail >= _head)
			return (_capacity - _tail) - ((_head == 0) ? 1 : 0);
		else // _tail < _head
			return _head - _tail - 1;
	}

	inline char* GetDirectEnqueuePtr() noexcept { return _buffer + _tail; }
	inline char* GetDirectDequeuePtr() noexcept { return _buffer + _head; }

	inline size_t DirectDequeueSize() const noexcept
	{
		return (_tail < _head) ? _capacity - _head : _tail - _head;
	}

	inline size_t MoveHead(size_t offset) noexcept {
		_head = (_head + offset) % _capacity;
		return offset;
	}

	inline size_t MoveTail(size_t offset) noexcept {
		_tail = (_tail + offset) % _capacity;
		return offset;
	}

	void ResizeBuffer(const size_t newCapacity) noexcept;

	inline bool IsEmpty() const noexcept { return _head == _tail; }
	inline bool IsFull() const noexcept { return ((_tail + 1) % _capacity) == _head; }
	inline void ClearBuffer() noexcept { _head = 0, _tail = 0; }
	inline size_t GetUsedSize() const noexcept
	{
		return (_head <= _tail) ? (_tail - _head) : (_capacity - _head + _tail);
	}
	inline size_t GetFreeSize() const noexcept
	{
		return _capacity - GetUsedSize() - 1;
	}

	size_t Peek(char* dst, size_t size) const noexcept;
	size_t Enqueue(const char* src, size_t size) noexcept;
	size_t Dequeue(char* dst, size_t size) noexcept;

	RingBuffer(size_t capacity = 1024) noexcept;
	~RingBuffer() noexcept;
};
