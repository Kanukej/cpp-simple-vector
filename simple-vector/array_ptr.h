#pragma once
#include <cassert>
#include <cstdlib>
#include <algorithm>

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) : size_(size) {
        if(size_ > 0) {
            raw_ptr_ = new Type[size_];
        }
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }
    
    ArrayPtr(const ArrayPtr&) = delete;
    
    ArrayPtr(ArrayPtr&& rhs) {
        raw_ptr_ = std::exchange(rhs.raw_ptr_, nullptr);
        size_ = std::exchange(rhs.size_, 0);    
    };

    ~ArrayPtr() {
        size_ = 0;
        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;
    ArrayPtr& operator=(ArrayPtr&& rhs) {
        raw_ptr_ = std::exchange(rhs.raw_ptr_, nullptr);
        size_ = std::exchange(rhs.size_, 0);
        return *this;
    }
    
    [[nodiscard]] Type* Release() noexcept {
        Type* tmp = raw_ptr_;
        raw_ptr_ =  nullptr;
        size_ = 0;
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
    
    size_t GetCapacity() const noexcept {
        return size_;
    }
    
    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
        std::swap(size_, other.size_);
    }

private:
    Type* raw_ptr_ = nullptr;
    size_t size_ = 0;
};
