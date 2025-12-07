#pragma once

// Packet.h 
// Serialization Buffer Structure 

class Packet {
	static constexpr const size_t BUFFER_CAPACITY = 256;  

private:
	char _buffer[BUFFER_CAPACITY];
	size_t _head = 0;
	size_t _tail = 0;

public:
	Packet() noexcept;
	~Packet() noexcept; 

	Packet(const Packet& other) noexcept;
	Packet& operator=(const Packet& other) noexcept;
	Packet(Packet&& other) noexcept;
	Packet& operator=(Packet&& other) noexcept; 

	inline const size_t GetCapacity() const noexcept { return BUFFER_CAPACITY; }
	inline size_t GetUsedSize() const noexcept { return _tail - _head; }
	inline size_t GetFreeSize() const noexcept { return BUFFER_CAPACITY - _tail; }
	inline void SetUsedSize(size_t size) noexcept {
		if (size > BUFFER_CAPACITY) size = BUFFER_CAPACITY;
		_head = 0;
		_tail = size;
	}
	inline const char* GetBuffer() const noexcept { return _buffer; } 
	inline char* GetBuffer() noexcept { return _buffer; } 
	inline void Clear() noexcept { _head = 0; _tail = 0; }

	// Header Fields Accessors 
	inline const uint8_t GetCode() const noexcept { return static_cast<uint8_t>(_buffer[0]); }
	inline const uint8_t GetSize() const noexcept { return static_cast<uint8_t>(_buffer[1]); }
	inline const Type GetType() const noexcept { return static_cast<Type>(_buffer[2]); }

	inline void SkipHeader() noexcept { _head = sizeof(PacketHeader); } 

	template<typename T>
	inline void Put(const T& val) noexcept {
		size_t pos = _tail;
		memcpy(_buffer + pos, &val, sizeof(T));
		_tail = pos + sizeof(T);
	}

	template<typename T>
	inline void Get(T& val) noexcept {
		size_t pos = _head;
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
