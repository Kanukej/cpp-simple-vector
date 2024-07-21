#pragma once
#include <cassert>
#include <cstdlib>
#include <algorithm>

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    template <typename FillFunc>
    explicit ArrayPtr(size_t size, FillFunc fill) : ArrayPtr(size) {
        fill(Get(), Get() + size); 
    }

    explicit ArrayPtr(size_t size) {
        if(size > 0) {
            raw_ptr_ = new Type[size];
        }
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }
    
    ArrayPtr(const ArrayPtr&) = delete;
    
    ArrayPtr(ArrayPtr&& rhs) {
        raw_ptr_ = std::exchange(rhs.raw_ptr_, nullptr);
    };

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;
    ArrayPtr& operator=(ArrayPtr&& rhs) {
        raw_ptr_ = std::exchange(rhs.raw_ptr_, nullptr);
        return *this;
    }
    
    [[nodiscard]] Type* Release() noexcept {
        Type* tmp = raw_ptr_;
        raw_ptr_ =  nullptr;
        return tmp;
    }

    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }
    
    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }

    explicit operator bool() const {
        return raw_ptr_;
    }

    Type* Get() const noexcept {
        return  raw_ptr_;
    }
    
    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};
