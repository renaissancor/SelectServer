#pragma once

// WinAtomic.h 

namespace Win {
    class Atomic32 {
    private:
        alignas(64) volatile LONG _value;

    public:
        explicit Atomic32(LONG val = 0) noexcept : _value(val) {}
        ~Atomic32() = default;

        Atomic32(const Atomic32&) = delete;
        Atomic32& operator=(const Atomic32&) = delete;

        inline LONG Load() const noexcept { return LoadRelaxed(); }
        inline void Store(LONG v) noexcept { StoreRelease(v); }

        inline LONG LoadRelaxed() const noexcept { return _value; }
        inline LONG LoadAcquire() const noexcept {
            return InterlockedCompareExchange(const_cast<volatile LONG*>(&_value), 0, 0);
        }

        inline void StoreRelaxed(LONG v) noexcept { _value = v; }
        inline void StoreRelease(LONG v) noexcept {
            InterlockedExchange(const_cast<volatile LONG*>(&_value), v);
        }

        inline LONG Increment() noexcept { return InterlockedIncrement(&_value); }
        inline LONG Decrement() noexcept { return InterlockedDecrement(&_value); }
        inline LONG Exchange(LONG val) noexcept { return InterlockedExchange(&_value, val); }
        inline LONG CompareExchange(LONG val, LONG comparand) noexcept {
            return InterlockedCompareExchange(&_value, val, comparand);
        }

        inline operator LONG() const noexcept { return Load(); }
        inline Atomic32& operator=(LONG val) noexcept { Store(val); return *this; }
        inline bool operator==(LONG val) const noexcept { return Load() == val; }
        inline bool operator!=(LONG val) const noexcept { return Load() != val; }

        inline LONG operator++() noexcept { return Increment(); }
        inline LONG operator--() noexcept { return Decrement(); }
        inline LONG operator++(int) noexcept { return InterlockedExchangeAdd(&_value, 1); }
        inline LONG operator--(int) noexcept { return InterlockedExchangeAdd(&_value, -1); }
        inline LONG operator+=(LONG val) noexcept { return InterlockedExchangeAdd(&_value, val) + val; }
        inline LONG operator-=(LONG val) noexcept { return InterlockedExchangeAdd(&_value, -val) - val; }
    };


    class Atomic64 {
    private:
        alignas(64) volatile LONGLONG _value;

    public:
        explicit Atomic64(LONGLONG val = 0) noexcept : _value(val) {}
        inline ~Atomic64() = default;
        Atomic64(const Atomic64&) = delete;
        Atomic64& operator=(const Atomic64&) = delete;

        inline LONGLONG Load() const noexcept { return LoadRelaxed(); }
        inline void Store(LONGLONG v) noexcept { StoreRelease(v); }

        inline LONGLONG LoadRelaxed() const noexcept { return _value; }
        inline LONGLONG LoadAcquire() const noexcept {
            return InterlockedCompareExchange64(const_cast<volatile LONGLONG*>(&_value), 0, 0);
        }

        inline void StoreRelaxed(LONGLONG v) noexcept { _value = v; }
        inline void StoreRelease(LONGLONG v) noexcept {
            InterlockedExchange64(const_cast<volatile LONGLONG*>(&_value), v);
        }

        inline LONGLONG Increment() noexcept { return InterlockedIncrement64(&_value); }
        inline LONGLONG Decrement() noexcept { return InterlockedDecrement64(&_value); }
        inline LONGLONG Exchange(LONGLONG val) noexcept { return InterlockedExchange64(&_value, val); }
        inline LONGLONG CompareExchange(LONGLONG val, LONGLONG comparand) noexcept {
            return InterlockedCompareExchange64(&_value, val, comparand);
        }

        inline operator LONGLONG() const noexcept { return Load(); }
        inline Atomic64& operator=(LONGLONG val) noexcept { Store(val); return *this; }
        inline bool operator==(LONGLONG val) const noexcept { return Load() == val; }
        inline bool operator!=(LONGLONG val) const noexcept { return Load() != val; }

        inline LONGLONG operator++() noexcept { return Increment(); }
        inline LONGLONG operator--() noexcept { return Decrement(); }
        inline LONGLONG operator++(int) noexcept { return InterlockedExchangeAdd64(&_value, 1); }
        inline LONGLONG operator--(int) noexcept { return InterlockedExchangeAdd64(&_value, -1); }
        inline LONGLONG operator+=(LONGLONG val) noexcept { return InterlockedExchangeAdd64(&_value, val) + val; }
        inline LONGLONG operator-=(LONGLONG val) noexcept { return InterlockedExchangeAdd64(&_value, -val) - val; }
    };


    class AtomicPtr {
    private:
        alignas(64) void* volatile _value;
    public:
        explicit AtomicPtr(void* val = nullptr) noexcept : _value(val) {}
        inline ~AtomicPtr() = default;
        AtomicPtr(const AtomicPtr&) = delete;
        AtomicPtr& operator=(const AtomicPtr&) = delete;

        inline void* Load() const noexcept { return LoadRelaxed(); }
        inline void Store(void* v) noexcept { StoreRelease(v); }

        inline void* LoadRelaxed() const noexcept { return _value; }
        inline void* LoadAcquire() const noexcept {
            return InterlockedCompareExchangePointer(const_cast<void**>(&_value), nullptr, nullptr);
        }

        inline void StoreRelaxed(void* v) noexcept { _value = v; }

        inline void StoreRelease(void* v) noexcept {
            InterlockedExchangePointer(const_cast<void**>(&_value), v);
        }

        inline void* Exchange(void* val) noexcept { return InterlockedExchangePointer(&_value, val); }
        inline void* CompareExchange(void* val, void* comparand) noexcept {
            return InterlockedCompareExchangePointer(&_value, val, comparand);
        }

        inline operator void* () const noexcept { return Load(); }
        inline AtomicPtr& operator=(void* val) noexcept { Store(val); return *this; }
    };
} // namespace Win

