#pragma once

// WinAtomic.h 

template<typename T>
class WinAtomic;

template<>
class WinAtomic<int> {
private:
    alignas(64) volatile LONG _value;

public:
    explicit WinAtomic(LONG val = 0) noexcept : _value(val) {}
    inline ~WinAtomic() = default;
    WinAtomic(const WinAtomic&) = delete;
    WinAtomic& operator=(const WinAtomic&) = delete;

    inline LONG Load() const noexcept {
        return InterlockedExchangeAdd(const_cast<volatile LONG*>(&_value), 0);
    }
    inline void Store(LONG val) noexcept { InterlockedExchange(&_value, val); }
    inline LONG Increment() noexcept { return InterlockedIncrement(&_value); }
    inline LONG Decrement() noexcept { return InterlockedDecrement(&_value); }
    inline LONG Exchange(LONG val) noexcept { return InterlockedExchange(&_value, val); }
    inline LONG CompareExchange(LONG val, LONG comparand) noexcept {
        return InterlockedCompareExchange(&_value, val, comparand);
    } // Returns previous value, sets to val if current == comparand 

    inline operator int() const noexcept { return Load(); }
    // inline int operator=(int val) noexcept { Store(val); return val; } // incorrect, should return reference to *this
    inline WinAtomic& operator=(int val) noexcept { Store(val); return *this; } // std::atomic
    inline bool operator==(int val) const noexcept { return Load() == val; }
    inline bool operator!=(int val) const noexcept { return Load() != val; }

    // Prefix Increment / Decrement
    inline int operator++() noexcept { return Increment(); }
    inline int operator--() noexcept { return Decrement(); }

    // Postfix Increment / Decrement (returns previous value)
    inline int operator++(int) noexcept { return InterlockedExchangeAdd(&_value, 1); }
    inline int operator--(int) noexcept { return InterlockedExchangeAdd(&_value, -1); }

    inline int operator+=(int val) noexcept { return InterlockedExchangeAdd(&_value, val) + val; }
    inline int operator-=(int val) noexcept { return InterlockedExchangeAdd(&_value, -val) - val; }
};

template<>
class WinAtomic<long long> {
private:
    alignas(64) volatile LONGLONG _value;

public:
    explicit inline WinAtomic(long long val = 0) noexcept : _value(val) {}
    inline ~WinAtomic() = default;
    WinAtomic(const WinAtomic&) = delete;
    WinAtomic& operator=(const WinAtomic&) = delete;

    inline LONGLONG Load() const noexcept {
        return InterlockedExchangeAdd64(const_cast<volatile LONGLONG*>(&_value), 0);
    }
    inline void Store(LONGLONG val) noexcept { InterlockedExchange64(&_value, val); }
    inline LONGLONG Increment() noexcept { return InterlockedIncrement64(&_value); }
    inline LONGLONG Decrement() noexcept { return InterlockedDecrement64(&_value); }
    inline LONGLONG Exchange(LONGLONG val) noexcept { return InterlockedExchange64(&_value, val); }
    inline LONGLONG CompareExchange(LONGLONG val, LONGLONG comparand) noexcept {
        return InterlockedCompareExchange64(&_value, val, comparand);
    } // Returns previous value, sets to val if current == comparand
    inline operator long long() const noexcept { return Load(); }
    inline WinAtomic& operator=(long long val) noexcept { Store(val); return *this; } // std::atomic
    inline bool operator==(long long val) const noexcept { return Load() == val; }
    inline bool operator!=(long long val) const noexcept { return Load() != val; }
    // Prefix Increment / Decrement
    inline long long operator++() noexcept { return Increment(); }
    inline long long operator--() noexcept { return Decrement(); }
    // Postfix Increment / Decrement (returns previous value)
    inline long long operator++(int) noexcept { return InterlockedExchangeAdd64(&_value, 1); }
    inline long long operator--(int) noexcept { return InterlockedExchangeAdd64(&_value, -1); }
    inline long long operator+=(long long val) noexcept { return InterlockedExchangeAdd64(&_value, val) + val; }
    inline long long operator-=(long long val) noexcept { return InterlockedExchangeAdd64(&_value, -val) - val; }
};

