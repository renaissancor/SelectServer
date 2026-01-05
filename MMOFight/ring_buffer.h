#pragma once

// ring buffer 

class ring_buffer {
	constexpr static int DEFAULT_CAPACITY = 4096; 

private:
	char* _buffer = nullptr; 
	int _capacity = 0;
	int _mask = 0; 
	int _head = 0;
	int _tail = 0;

private:
	void resize_buffer(const int new_capacity) noexcept; 
	void ensure_capacity(const int size) noexcept; 

public:
	// inline constant functions 
	inline bool empty() const noexcept { return _head == _tail; } 
	inline bool full() const noexcept { return ((_tail + 1) & _mask ) == _head; } 
	inline int get_head() const noexcept { return _head; }
	inline int get_tail() const noexcept { return _tail; }
	inline int get_capacity() const noexcept { return _capacity; } 
	inline int get_used_size() const noexcept 
		{ return (_head <= _tail) ? (_tail - _head) : (_capacity - _head + _tail); }
	inline int get_free_size() const noexcept 
		{ return _capacity - get_used_size() - 1; }
	inline int direct_enqueue_size() const noexcept {
		if (_tail >= _head) {
			int space = _capacity - _tail;
			if (_head == 0) space -= 1; 
			return space;
		}
		else {
			return _head - _tail - 1;
		}
	}
	inline int direct_dequeue_size() const noexcept {
		return (_tail < _head) ? _capacity - _head : _tail - _head;
	}

	// inline modifier functions 
	inline void clear_buffer() noexcept { _head = 0; _tail = 0; }
	inline int move_head(int offset) noexcept {
		_head = (_head + offset) & _mask;
		return offset;
	}
	inline int move_tail(int offset) noexcept {
		_tail = (_tail + offset) & _mask;
		return offset;
	}

	inline const char* get_head_ptr() const noexcept { return (_buffer + _head); }
	inline const char* get_tail_ptr() const noexcept { return (_buffer + _tail); }
	inline char* get_tail_ptr() noexcept { return (_buffer + _tail); }

	// functions 
	ring_buffer(int capacity) noexcept;
	~ring_buffer();
	ring_buffer(const ring_buffer&) = delete;
	ring_buffer& operator=(const ring_buffer&) = delete;
	int peek(char* dst, int bytes) const noexcept;
	int enqueue(const char* src, int size) noexcept;
	int dequeue(char* dst, int size) noexcept;
};
