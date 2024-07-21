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

    explicit SimpleVector(size_t size) {
        Initialize(size, [](Type* begin, Type* end) {
            std::fill(begin, end, Type {});
        });
    }

    SimpleVector(size_t size, const Type& value) {
        Initialize(size, [&value](Type* begin, Type* end) {
            std::fill(begin, end, value);
        });
    }

    SimpleVector(std::initializer_list<Type> init) {
        Initialize(init.size(), [&init](Type* begin, Type* end) {
            auto valueIt = std::make_move_iterator(init.begin());
            for (Type* it = begin; it != end; ++it, ++valueIt) {
                *it = *valueIt;
            }
        });
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
                    std::exchange(t, Type {});
                });
            } catch (std::bad_alloc&) {
                throw;
            }
            size_ = new_size;
        } else {
            Initialize(
                new_size > GetCapacity() * 2 ? new_size : GetCapacity() * 2,
                [new_size, b_ = this->begin(), e_ = this->end()](Type* begin, Type* end) {
                    end = std::copy(
                        std::make_move_iterator(b_),
                        std::make_move_iterator(e_), 
                        begin
                    );
                    std::for_each(end, begin + new_size, [](Type& t) {
                        std::exchange(t, Type {});
                    });
                }
            );
            Resize(new_size);
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
    
    SimpleVector(const SimpleVector& other) {
        if (&other == this) {
            return;
        }
        SimpleVector tmp;
        tmp.Initialize(other.GetSize(), [&other](Type* begin, Type* end) {
            auto valueIt = other.begin();
            for (Type* it = begin; it != end; ++it, ++valueIt) {
                *it = *valueIt;
            }
        });
        this->swap(tmp);
    }
    
    SimpleVector(SimpleVector&& other) {
        if (&other == this) {
            return;
        }
        this->data_ = std::exchange(other.data_, {});
        this->size_ = std::exchange(other.size_, 0);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (&rhs == this) {
            return *this;
        }
        SimpleVector tmp;
        tmp.Initialize(rhs.GetSize(), [&rhs](Type* begin, Type* end) {
            auto valueIt = rhs.cbegin();
            for (Type* it = begin; it != end; ++it, ++valueIt) {
                *it = *valueIt;
            }
        });
        this->swap(tmp);
        return *this;
    }
    
    SimpleVector& operator=(SimpleVector&& other) {
        if (&other == this) {
            return *this;
        }
        this->data_ = std::exchange(other.data_, {});
        this->size_ = std::exchange(other.size_, 0);
        return *this;
    }
    
    void PushBack(const Type& item) {
        ResizeIfNeeded();
        data_[size_] = std::move(item);
        ++size_;
    }
    
    void PushBack(Type&& item) {
        ResizeIfNeeded();
        data_[size_] = std::move(item);
        ++size_;
    }
   
    void PopBack() noexcept {
        if (GetSize() > 0) {
            --size_;
        }
    }
    
    Iterator Insert(ConstIterator pos, const Type& value) {
        Insert(Iterator(pos), Type { value });
    }
    
    Iterator Insert(Iterator pos, Type&& value) {
        size_t sz = GetSize();
        Iterator it = begin();
        if (sz == GetCapacity()) {
            Initialize(
                GetCapacity() ? 2 * sz : 1,
                [&it, &sz, p_ = pos, e_ = end(), b_ = begin()](Type* begin, Type* end) {
                    if (begin < end) {
                        std::copy(
                            std::make_move_iterator(b_),
                            std::make_move_iterator(p_), 
                            begin
                        );
                        it = std::copy_backward(
                            std::make_move_iterator(p_), 
                            std::make_move_iterator(e_), 
                            begin + sz + 1
                        );
                    }
                }
            );
            size_ = sz + 1;
        } else {
            it = std::copy_backward(
                std::make_move_iterator(pos),
                std::make_move_iterator(end()),
                end() + 1
            );
            ++size_;
        }
        *(--it) = std::exchange(value, {});
        return it;
    }
    
    Iterator Erase(ConstIterator pos) {
        size_t id = pos - begin();
        std::copy(
            std::make_move_iterator(begin() + id + 1),
            std::make_move_iterator(end()), 
            begin() + id
        );
        --size_;
        return begin() + id;
    }
    
    void swap(SimpleVector& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    void Reserve(size_t size) {
        size_t sz = GetSize();
        if (size > GetCapacity()) {
            Initialize(size, [b_ = cbegin(), e_ = cend()](Type* begin, Type* end) {
                if (begin < end) {
                    std::copy(b_, e_, begin);            
                }
            });
            size_ = sz;
        }
    }
    
private:
    void ResizeIfNeeded() {
        size_t sz = GetSize();
        if (sz == GetCapacity()) {
            Initialize(
                GetCapacity() ? 2 * sz : 1,
                [b_ = this->begin(), e_ = this->end()](Type* begin, Type* end) {
                    if (begin < end) {
                        std::copy(
                            std::make_move_iterator(b_),
                            std::make_move_iterator(e_),
                            begin
                        );
                    }
                }
            );
            size_ = sz;
        }
    }
    
    template <typename FillFunc>
    void Initialize(size_t size, FillFunc fill) {
        if (size > 0) {
            try {
                ArrayPtr<Type> tmp(size, fill);
                data_.swap(tmp);
                size_ = size;
                capacity_ = size;
            } catch (std::bad_alloc&) {
                throw;
            }
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
