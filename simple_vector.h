#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <functional>

#include "array_ptr.h"

struct ReserveProxyObj {
    size_t value = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return { capacity_to_reserve };
}

template <typename Type>
class SimpleVector {

    using Iterator = Type*;
    using ConstIterator = const Type*;

public:

    SimpleVector() noexcept = default;

    SimpleVector(const ReserveProxyObj& reserve_obj) : SimpleVector() {
        size_t capacity = reserve_obj.value;
        if (capacity == 0) {
            return;
        }
        Reserve(capacity);
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        ArrayPtr<Type> temp(size);
        data_.swap(temp);
        size_ = capacity_ = size;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : SimpleVector(size) {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : SimpleVector(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }

    //Конструктор копирования
    SimpleVector(const SimpleVector& other) {
        CopyAndSwap(other);
    }

    //Конструктор перемещения
    SimpleVector(SimpleVector&& other) {
        MoveFrom(other);
    }

    SimpleVector& operator=(SimpleVector&& other) {
        MoveFrom(other);
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return data_[index];
    }

    // Возвращает ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        return const_cast<Type&>(
                const_cast<const SimpleVector*>(this)->At(index)
        );
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        using namespace std::literals;
        if (index >= size_) {
            throw std::out_of_range("out of range"s);
        }

        return data_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(const size_t new_size) {
        //new_size меньше или равен вместимости capacity_, но больше, чем size_
        if (new_size > size_ && new_size <= capacity_) {
            std::fill(end(), begin() + new_size, Type{});
        } else { //Превышает вместимость
            Reserve(new_size);
        }
        size_ = new_size;
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ != 0 && capacity_ >= new_capacity) {
            return;
        }
        new_capacity = std::max(new_capacity, size_t(1));
        ArrayPtr<Type> temp(new_capacity);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), temp.Get());
        data_.swap(temp);
        capacity_ = new_capacity;
    }

    void PushBack(const Type& value) {
        if (size_ == capacity_) {
            Reserve(capacity_ * 2);
        }
        data_[size_] = value;
        ++size_;
    }

    //Перемещающий push_back
    void PushBack(Type&& value) {
        if (size_ == capacity_) {
            Reserve(capacity_ * 2);
        }
        data_[size_] = std::move(value);
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        int offset = pos - begin();
        Iterator true_position = begin() + offset;
        if (size_ == capacity_) {
            Reserve(capacity_ * 2);
            true_position = begin() + offset;
        }

        std::move_backward(std::make_move_iterator(true_position), std::make_move_iterator(end()),
                end() + 1);
        *true_position = value;
        ++size_;
        return true_position;
    }

    //Перемещающий insert
    Iterator Insert(ConstIterator pos, Type&& value) {
        int offset = pos - begin();
        Iterator true_position = begin() + offset;
        if (size_ == capacity_) {
            Reserve(capacity_ * 2);
            true_position = begin() + offset;
        }

        std::move_backward(std::make_move_iterator(true_position), std::make_move_iterator(end()),
                           end() + 1);
        *true_position = std::move(value);
        ++size_;
        return true_position;
    }

    Iterator Erase(ConstIterator pos) {
        int offset = pos - begin();
        Iterator true_position = begin() + offset;
        std::move(std::make_move_iterator(true_position + 1), std::make_move_iterator(end()),
                true_position);
        --size_;
        return true_position;
    }

    void PopBack() noexcept {
        size_ = size_ > 0 ? size_ - 1 : 0;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return data_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return data_.Get() + size_;
    }

    SimpleVector& operator=(const SimpleVector& other) {
        CopyAndSwap(other);
        return *this;
    }

    void swap(SimpleVector& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:

    //Сам массив
    ArrayPtr<Type> data_ = {};

    //Размер массива
    size_t size_ = 0;

    //Вместимость массива
    size_t capacity_ = 0;

    void CopyAndSwap(const SimpleVector& other) {
        size_t size = other.size_;
        ArrayPtr<Type> temp(size);
        std::copy(other.begin(), other.end(), temp.Get());
        data_.swap(temp);
        size_ = capacity_ = size;
    }

    void MoveFrom(SimpleVector& other) {
        data_ = std::move(other.data_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

};


template <typename Type>
bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs > lhs);
}
