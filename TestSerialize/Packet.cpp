#include "stdafx.h"
#include "Packet.h" 

Packet::Packet() noexcept
	: _head(0), _tail(0), _buffer{} 
{
	// memset(_buffer, 0, BUFFER_CAPACITY); // Not necessary to reset buffer 
}

Packet::~Packet() noexcept 
{
}

Packet::Packet(const Packet& other) noexcept
	: _head(other._head), _tail(other._tail)
{
	_head = other._head;
	_tail = other._tail;
	memcpy(_buffer, other._buffer, BUFFER_CAPACITY);
}

Packet& Packet::operator=(const Packet& other) noexcept
{
	if (this != &other) {
		_head = other._head;
		_tail = other._tail;
		memcpy(_buffer, other._buffer, other._tail);
	}
	return *this;
}

Packet::Packet(Packet&& other) noexcept
	: _head(other._head), _tail(other._tail)
{
	memcpy(_buffer, other._buffer, BUFFER_CAPACITY);
	other._head = 0;
	other._tail = 0;
	memset(other._buffer, 0, BUFFER_CAPACITY);
}

Packet& Packet::operator=(Packet&& other) noexcept
{
	if (this != &other) {
		_head = other._head;
		_tail = other._tail;
		memcpy(_buffer, other._buffer, BUFFER_CAPACITY);
		other._head = 0;
		other._tail = 0;
		memset(other._buffer, 0, BUFFER_CAPACITY);
	}
	return *this;
}
