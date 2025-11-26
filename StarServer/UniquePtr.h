#pragma once

// UniquePtr.h 

template<typename T>
class UniquePtr {
private:
	T* _ptr;
public:
	explicit UniquePtr(T* p = nullptr) : _ptr(p) {}
	~UniquePtr() noexcept { delete _ptr; }
	UniquePtr(const UniquePtr&) = delete;
	UniquePtr& operator=(const UniquePtr&) = delete;

	UniquePtr(UniquePtr&& other) noexcept : _ptr(other._ptr) {
		other._ptr = nullptr;
	}
	UniquePtr& operator=(UniquePtr&& other) noexcept {
		if (this == &other) return *this;
		if (_ptr) delete _ptr;
		_ptr = other._ptr;
		other._ptr = nullptr;
		return *this;
	}

	inline T* Get() const noexcept { return _ptr; }
	T& operator*() const noexcept { return *_ptr; }
	T* operator->() const noexcept { return _ptr; }
	inline void Reset(T* p = nullptr) noexcept {
		if (_ptr == p) return;
		delete _ptr;
		_ptr = p;
	}
	T* Release() noexcept {
		T* temp = _ptr;
		_ptr = nullptr;
		return temp;
	}
	void Swap(UniquePtr& other) noexcept {
		T* temp = _ptr;
		_ptr = other._ptr;
		other._ptr = temp;
	}
	explicit operator bool() const noexcept {
		return _ptr != nullptr;
	}
};
