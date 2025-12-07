#pragma once

// Serialization Buffer Structure 
// Goal is to implement RPC (Remote Procedure Call) mechanism later 

class Packet {
	static constexpr const int BUFFER_CAPACITY = 256 - sizeof(int) * 2; // 2^8, uint8_t size, specified in header 
private:
	int _head = 0;
	int _tail = 0;
	char _buffer[BUFFER_CAPACITY];

public:
	Packet() = default;
	~Packet() = default;

	Packet(const Packet& other) = delete;
	Packet& operator=(const Packet& other) = delete;
	Packet(Packet&& other) = delete;
	Packet& operator=(Packet&& other) = delete; 

	inline const int GetCapacity() const noexcept { return BUFFER_CAPACITY; }
	inline int GetUsedSize() const noexcept { return _tail - _head; }
	inline int GetFreeSize() const noexcept { return BUFFER_CAPACITY - _tail; }
	inline void Clear() noexcept { _head = 0; _tail = 0; } 

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
