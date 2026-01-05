#include <cstring> 
#include "ring_buffer.h"

static inline int min(int a, int b) noexcept { return (a < b) ? a : b; } 

ring_buffer::ring_buffer(int capacity) noexcept :
	_capacity(capacity), _mask(capacity - 1), _head(0), _tail(0), _buffer(nullptr)
{
	int new_capacity = DEFAULT_CAPACITY;
	while (new_capacity < capacity) new_capacity <<= 1;
	_capacity = new_capacity; 
	_mask = new_capacity - 1; 
	_buffer = new char[_capacity]; 
}

ring_buffer::~ring_buffer() {
	if (_buffer) delete[] _buffer;
}

void ring_buffer::resize_buffer(const int new_capacity) noexcept {
	if (new_capacity <= _capacity) return;
	char* new_buffer = new char[new_capacity];
	int used_size = get_used_size();
	peek(new_buffer, used_size);
	delete[] _buffer;
	_buffer = new_buffer;
	_capacity = new_capacity;
	_mask = new_capacity - 1; 
	_head = 0;
	_tail = used_size;
}

void ring_buffer::ensure_capacity(int write_size) noexcept {
	int need = get_used_size() + write_size + 1;
	if (need <= _capacity) return;
	int new_cap = _capacity;
	while (new_cap < need)
		new_cap <<= 1;
	resize_buffer(new_cap);
}


int ring_buffer::peek(char* dst, int bytes) const noexcept {
	if (bytes > get_used_size()) bytes = get_used_size();
	int first_chunk = min(bytes, _capacity - _head);
	memcpy_s(dst, bytes, _buffer + _head, first_chunk);
	int remaining = bytes - first_chunk;
	if (remaining > 0) {
		memcpy_s(dst + first_chunk, bytes - first_chunk, _buffer, remaining);
	}
	return bytes;
}

int ring_buffer::enqueue(const char* src, int enq_size) noexcept {
	if (enq_size > get_free_size()) ensure_capacity(enq_size); // (_capacity + size);
	int first_chunk = min(enq_size, direct_enqueue_size());
	memcpy_s(_buffer + _tail, _capacity - _tail, src, first_chunk);
	int remaining = enq_size - first_chunk;
	if (remaining == 0) {
		_tail = (_tail + first_chunk) & _mask;
	}
	else {
		memcpy_s(_buffer, _capacity, src + first_chunk, remaining);
		_tail = remaining;
	}
	return enq_size;
}

int ring_buffer::dequeue(char* dst, int deq_size) noexcept {
	if (deq_size > get_used_size()) deq_size = get_used_size();
	int first_chunk = min(deq_size, _capacity - _head);
	memcpy_s(dst, deq_size, _buffer + _head, first_chunk);
	int remaining = deq_size - first_chunk;
	if (remaining == 0) {
		_head = (_head + first_chunk) & _mask;
	}
	else {
		memcpy_s(dst + first_chunk, deq_size - first_chunk, _buffer, remaining);
		_head = remaining;
	}
	return deq_size;
}


// Test ring_buffer functionality

/*

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "ring_buffer.h"

void test_basic_flow() {
    std::cout << "Testing Basic Flow..." << std::endl;
    ring_buffer rb(64); // 실제로는 64로 생성됨

    std::string msg = "Hello Ring Buffer!";
    rb.enqueue(msg.c_str(), (int)msg.size());

    char out[128] = { 0 };
    rb.dequeue(out, (int)msg.size());

    assert(std::string(out) == msg);
    assert(rb.empty());
    std::cout << "Success!" << std::endl;
}

void test_wrap_around() {
    std::cout << "Testing Wrap-around..." << std::endl;
    ring_buffer rb(32); // 초기 32

    // 20바이트 채움
    rb.enqueue("01234567890123456789", 20);
    // 15바이트 빼냄 (head가 중간에 위치)
    char temp[20];
    rb.dequeue(temp, 15);

    // 이제 남은 공간은 작지만, Wrap-around를 통해 데이터를 더 넣음
    std::string long_msg = "This message will wrap around the buffer";
    rb.enqueue(long_msg.c_str(), (int)long_msg.size()); // 여기서 resize 발생 가능

    char out[100] = { 0 };
    int deq_size = rb.dequeue(out, (int)rb.get_used_size());

    assert(deq_size > 0);
    std::cout << "Success!" << std::endl;
}

void test_direct_access() {
    std::cout << "Testing Direct Access (Send/Recv Simulation)..." << std::endl;
    ring_buffer rb(128);

    // 1. Recv 시나리오 (버퍼에 직접 쓰기)
    int can_write = rb.direct_enqueue_size();
    char* write_ptr = rb.get_tail_ptr();

    // 가상의 데이터 수신
    const char* mock_data = "Direct Data";
    int mock_recv_size = (int)strlen(mock_data);
    memcpy(write_ptr, mock_data, mock_recv_size);
    rb.move_tail(mock_recv_size);

    // 2. Send 시나리오 (버퍼에서 직접 읽기)
    int can_read = rb.direct_dequeue_size();
    const char* read_ptr = rb.get_head_ptr();

    assert(memcmp(read_ptr, mock_data, mock_recv_size) == 0);
    rb.move_head(mock_recv_size);

    assert(rb.empty());
    std::cout << "Success!" << std::endl;
}

void test_resize_integrity() {
    std::cout << "Testing Resize Integrity..." << std::endl;
    ring_buffer rb(32);

    std::string pattern = "ABCDE";
    for (int i = 0; i < 10; ++i) {
        rb.enqueue(pattern.c_str(), 5);
    }
    // 현재 50바이트 들어가 있음 (최초 32에서 resize 발생)

    assert(rb.get_used_size() == 50);
    assert(rb.get_capacity() >= 64);

    char out[60] = { 0 };
    rb.dequeue(out, 50);

    for (int i = 0; i < 10; ++i) {
        assert(memcmp(out + (i * 5), "ABCDE", 5) == 0);
    }
    std::cout << "Success!" << std::endl;
}

int main() {
    try {
        for (int i = 0; i < 10; ++i) {
            std::cout << "\n=== Test Round " << (i + 1) << " ===" << std::endl;
            test_basic_flow();
            test_wrap_around();
            test_direct_access();
            test_resize_integrity();
        }

        std::cout << "\nAll Tests Passed Successfully!" << std::endl;
    }
    catch (...) {
        std::cerr << "Test failed with unknown exception." << std::endl;
    }
    return 0;
}

*/
