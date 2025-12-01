#pragma once

class RingBuffer {
protected:
	char* _buffer = nullptr;
	int _capacity = 0;
	int _head = 0;
	int _tail = 0;

protected:
	inline int GetHeadIndex() const noexcept { return _head; }
	inline int GetTailIndex() const noexcept { return _tail; }
	inline int GetCapacity() const noexcept { return _capacity; }

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

	void ResizeBuffer(const int newCapacity) noexcept;

public:
	inline bool IsEmpty() const noexcept { return _head == _tail; }
	inline bool IsFull() const noexcept { return ((_tail + 1) % _capacity) == _head; }
	inline void ClearBuffer() noexcept { _head = 0, _tail = 0; }
	inline int GetUsedSize() const noexcept
	{ return (_head <= _tail) ? (_tail - _head) : (_capacity - _head + _tail); }
	inline int GetFreeSize() const noexcept
	{ return _capacity - GetUsedSize() - 1; }

	int Peek(char* dst, int size) const noexcept;
	int Enqueue(const char* src, int size) noexcept;
	int Dequeue(char* dst, int size) noexcept;

	RingBuffer(int capacity = 1024) noexcept;
	~RingBuffer() noexcept;
}; 

class SendRingBuffer : public RingBuffer {
public:
	SendRingBuffer(int capacity = 1024) : RingBuffer(capacity) {}

	using RingBuffer::IsEmpty;
	using RingBuffer::IsFull;
	using RingBuffer::GetUsedSize;
	using RingBuffer::GetFreeSize;
	using RingBuffer::ClearBuffer;
	using RingBuffer::Peek;
	using RingBuffer::Enqueue;

	int SendTCP(SOCKET socket) noexcept; 
};

class RecvRingBuffer : public RingBuffer {
public:
	RecvRingBuffer(int capacity = 4096) : RingBuffer(capacity) {}

	using RingBuffer::IsEmpty;
	using RingBuffer::IsFull;
	using RingBuffer::GetUsedSize;
	using RingBuffer::GetFreeSize;
	using RingBuffer::ClearBuffer;
	using RingBuffer::Peek;
	using RingBuffer::Dequeue;

	int RecvTCP(SOCKET socket) noexcept;
}; 