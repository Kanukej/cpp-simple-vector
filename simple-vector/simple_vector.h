#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

class SimpleVectorProxy {
public:
    explicit SimpleVectorProxy(size_t sz) : size_(sz) {
        //
    }
    size_t GetSize() const {
        return size_;
    }
private:
    const size_t size_;
};

SimpleVectorProxy Reserve(size_t sz);

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    ~SimpleVector() {
        size_ = 0;
        capacity_ = 0;
    }
    
    SimpleVector(const SimpleVectorProxy& p_obj) {
        Reserve(p_obj.GetSize());
    }

    explicit SimpleVector(size_t size) : SimpleVector(size, Type {})  {
        //
    }

    SimpleVector(size_t size, const Type& value) : data_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init) : data_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::move(init.begin(), init.end(), begin());
    }

    size_t GetSize() const noexcept {
        return size_;
    }
    
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return data_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return data_[index];
    }

    Type& At(size_t index)  {
        if (index < size_) {
            return data_[index];
        } else {
            throw std::out_of_range("index is out of range");
        }
    }

    const Type& At(size_t index) const {
        if (index < size_) {
            return data_[index];
        } else {
            throw std::out_of_range("index is out of range");
        }
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= GetSize()) {
            size_ = new_size;
        } else if (new_size <= GetCapacity()) {
            try {
                std::for_each(end(), begin() + new_size, [](Type& t) {
                    t = Type {};
                });
            } catch (std::bad_alloc&) {
                throw;
            }
            size_ = new_size;
        } else {
            SimpleVector tmp;
            tmp.Reserve(new_size > GetCapacity() * 2 ? new_size : GetCapacity() * 2);
            tmp.size_ = new_size;
            std::for_each(
                std::move(begin(), end(), tmp.begin()),
                tmp.end(),
                [](Type& t) {
                    t = Type {};
                }
            );
            swap(tmp);
        }
    }

    Iterator begin() noexcept {
        return data_.Get();
    }

    Iterator end() noexcept {
        return begin() + size_;
    }

    ConstIterator begin() const noexcept {
        return data_.Get();
    }

    ConstIterator end() const noexcept {
        return begin() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return data_.Get();
    }

    ConstIterator cend() const noexcept {
        return cbegin() + size_;
    }
    
    SimpleVector(const SimpleVector& other) : data_(other.GetCapacity()), size_(other.GetSize()), capacity_(other.GetCapacity()) {
        if (&other == this) {
            return;
        }
        std::copy(other.begin(), other.end(), begin());
    }
    
    SimpleVector(SimpleVector&& other) {
        if (&other == this) {
            return;
        }
        this->data_ = std::exchange(other.data_, {});
        this->size_ = std::exchange(other.size_, 0);
        this->capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (&rhs == this) {
            return *this;
        }
        SimpleVector tmp(rhs);
        swap(tmp);
        return *this;
    }
    
    SimpleVector& operator=(SimpleVector&& other) {
        if (&other == this) {
            return *this;
        }
        this->data_ = std::exchange(other.data_, {});
        this->size_ = std::exchange(other.size_, 0);
        this->capacity_ = std::exchange(other.capacity_, 0);
        return *this;
    }
    
    void PushBack(const Type& item) {
        if (GetSize() == GetCapacity()) {
            SimpleVector tmp;
            tmp.Reserve(GetCapacity() > 0 ? 2 * GetCapacity() : 1);
            tmp.size_ = GetSize();
            std::copy(cbegin(), cend(), tmp.begin());
            tmp.PushBack(item);
            swap(tmp);
        } else {
            data_[size_] = item;
            ++size_;
        }
    }
    
    void PushBack(Type&& item) {    
        if (GetSize() == GetCapacity()) {
            SimpleVector tmp;
            tmp.Reserve(GetCapacity() > 0 ? 2 * GetCapacity() : 1);
            tmp.size_ = GetSize();
            std::move(begin(), end(), tmp.begin());
            tmp.PushBack(std::move(item));
            swap(tmp);
        } else {
            data_[size_] = std::move(item);
            ++size_;
        }
    }
   
    void PopBack() noexcept {
        if (GetSize() > 0) {
            --size_;
        }
    }
    
    Iterator Insert(ConstIterator pos, const Type& value) {
        Iterator ans;
        if (GetSize() == GetCapacity()) {
            SimpleVector tmp;
            tmp.Reserve(GetCapacity() ? 2 * GetSize() : 1);
            tmp.size_ = GetSize() + 1;
            std::copy(cbegin(), pos, tmp.begin());
            ans = std::copy_backward(pos, end(), tmp.end());
            *(--ans) = value;
            swap(tmp);
        } else {
            ans = std::copy_backward(pos, end(), end() + 1);
            *(--ans) = value;
            ++size_;
        }
        return ans;
    }
    
    Iterator Insert(Iterator pos, Type&& value) {    
        Iterator ans;
        if (GetSize() == GetCapacity()) {
            SimpleVector tmp;
            tmp.Reserve(GetCapacity() ? 2 * GetSize() : 1);
            tmp.size_ = GetSize() + 1;
            std::move(begin(), pos, tmp.begin());
            ans = std::move_backward(pos, end(), tmp.end());
            *(--ans) = std::move(value);
            swap(tmp);
        } else {
            ans = std::move_backward(pos, end(), end() + 1);
            *(--ans) = std::move(value);
            ++size_;
        }
        return ans;
    }
    
    Iterator Erase(ConstIterator pos) {
        size_t id = pos - begin();
        std::move(begin() + id + 1, end(), begin() + id);
        --size_;
        return begin() + id;
    }
    
    void swap(SimpleVector& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    void Reserve(size_t size) {
        if (size > GetCapacity()) {
            ArrayPtr<Type> tmp(size);
            data_.swap(tmp);
            capacity_ = size;
        }
    }

private:
    ArrayPtr<Type> data_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

SimpleVectorProxy Reserve(size_t sz) {
    return SimpleVectorProxy(sz);
}
