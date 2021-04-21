#pragma once

#include <cassert>
#include <cstdlib>
#include <initializer_list>

template <typename Type>
class ArrayPtr {
public:
    // Инициализирует ArrayPtr нулевым указателем
    ArrayPtr() = default;

    // Создаёт в куче массив из size элементов типа Type.
    // Если size == 0, поле raw_ptr_ должно быть равно nullptr
    explicit ArrayPtr(size_t size) {
        if (size == 0) {
            return;
        }
        raw_ptr_ = new Type[size]{};
    }

    //Инициализация значением value
    explicit ArrayPtr(size_t size, const Type& value) {
        if (size == 0) {
            return;
        }
        raw_ptr_ = new Type[size];
        std::fill(raw_ptr_, raw_ptr_ + size, value);
    }

    ArrayPtr(std::initializer_list<Type> init) {
        raw_ptr_ = new Type[init.size()];
        int counter = 0;
        for (auto it = init.begin(); it != init.end(); ++it) {
            raw_ptr_[counter] = *it;
            ++counter;
        }
    }

    // Конструктор из сырого указателя, хранящего адрес массива в куче либо nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }

    // Запрещаем копирование
    ArrayPtr(const ArrayPtr&) = delete;

    // Запрещаем присваивание
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    //Конструктор перемещения
    ArrayPtr(ArrayPtr&& other) noexcept {
        MoveFrom(other);
    }

    //Оператор перемещения
    ArrayPtr& operator=(ArrayPtr&& other) {
        MoveFrom(other);
        return *this;
    }

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    // Прекращает владением массивом в памяти, возвращает значение адреса массива
    // После вызова метода указатель на массив должен обнулиться
    [[nodiscard]] Type* Release() noexcept {
        Type* ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return ptr;
    }

    // Возвращает ссылку на элемент массива с индексом index
    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    // Возвращает константную ссылку на элемент массива с индексом index
    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    // Возвращает true, если указатель ненулевой, и false в противном случае
    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    // Возвращает значение сырого указателя, хранящего адрес начала массива
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // Обменивается значениям указателя на массив с объектом other
    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:

    Type* raw_ptr_ = nullptr;

    void MoveFrom(ArrayPtr& other) {
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);
    }

};
