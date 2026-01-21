#pragma once

#include "Types.h" 

// Serialization Buffer Structure 
// Goal is to implement RPC (Remote Procedure Call) mechanism later 

class Packet {
	static constexpr const i32 BUFFER_CAPACITY = 64;
private:
	i32 _head = 0;
	i32 _tail = 0;
	i08 _buffer[BUFFER_CAPACITY];

public:
	Packet() = default;
	~Packet() = default;

	Packet(const Packet& other) = delete;
	Packet& operator=(const Packet& other) = delete;

	Packet(Packet&& other) noexcept {
		_head = other._head;
		_tail = other._tail;
		memcpy(_buffer, other._buffer, BUFFER_CAPACITY);
	}

	Packet& operator=(Packet&& other) noexcept {
		if (this != &other) {
			_head = other._head;
			_tail = other._tail;
			memcpy(_buffer, other._buffer, BUFFER_CAPACITY);
		}
		return *this;
	}

	inline const i32 GetCapacity() const noexcept { return BUFFER_CAPACITY; }
	inline i32 GetUsedSize() const noexcept { return _tail - _head; }
	inline i32 GetFreeSize() const noexcept { return BUFFER_CAPACITY - _tail; }
	inline void Clear() noexcept { _head = 0; _tail = 0; }
	inline const i08* GetHeadPtr() const noexcept { return _buffer; } 

	template<typename T>
	inline void Put(const T& val) noexcept {
		int pos = _tail;
		memcpy(_buffer + pos, &val, sizeof(T));
		_tail = pos + sizeof(T);
	}

	template<typename T>
	inline void Get(T& val) noexcept {
		int pos = _head;
		memcpy(&val, _buffer + pos, sizeof(T));
		_head = pos + sizeof(T);
	}

	template<typename T>
	inline Packet& GetData(T& out) noexcept {
		Get(out);
		return *this;
	}

	template<typename T>
	inline Packet& PutData(const T& in) noexcept {
		Put(in);
		return *this;
	}

	template<typename T>
	inline Packet& operator<<(const T& v) noexcept {
		Put(v);
		return *this;
	}

	template<typename T>
	inline Packet& operator>>(T& v) noexcept {
		Get(v);
		return *this;
	}
};
