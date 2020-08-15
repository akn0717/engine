#include "collections.hxx"
#include "array.hxx"

// core::pod::Array members
//////////////////////////////////////////////////////////////////////////

template<typename T>
inline core::pod::Array<T>::Array(core::allocator &alloc) noexcept
    : _allocator{ &alloc }
{
}

template<typename T>
inline core::pod::Array<T>::Array(const Array<T> &other) noexcept
    : _allocator{ other._allocator }
{
    const auto other_size = other._size;

    array::set_capacity(*this, other_size);
    memcpy(_data, other._data, sizeof(T) * other_size);
    _size = other_size;
}

template<typename T>
inline core::pod::Array<T>::Array(Array<T> &&other) noexcept
    : _allocator{ other._allocator }
    , _data{ std::exchange(other._data, nullptr) }
    , _size{ std::exchange(other._size, 0) }
    , _capacity{ std::exchange(other._capacity, 0) }
{
}

template<typename T>
inline core::pod::Array<T>::~Array() noexcept
{
    if constexpr (std::is_const_v<T>)
    {
        // #todo should we allow allocating const data in the first place? or is it used only by array views?
        _allocator->deallocate(const_cast<std::remove_const_t<T>*>(_data));
    }
    else
    {
        _allocator->deallocate(_data);
    }
}

template<typename T>
auto core::pod::Array<T>::operator=(const Array<T> &other) noexcept -> Array<T> &
{
    const auto other_size = other._size;

    array::resize(*this, other_size);
    memcpy(_data, other._data, sizeof(T) * other_size);
    return *this;
}

template<typename T>
inline auto core::pod::Array<T>::operator=(Array && other) noexcept -> Array &
{
    if (this != &other)
    {
        IS_ASSERT(_allocator == other._allocator, "Allocators differ on array objects!");
        std::swap(_data, other._data);
        _capacity = std::exchange(other._capacity, 0);
        _size = std::exchange(other._size, 0);
    }
    return *this;
}

template<typename T>
inline auto core::pod::Array<T>::operator[](uint32_t i) -> T &
{
    return _data[i];
}

template<typename T>
inline auto core::pod::Array<T>::operator[](uint32_t i) const -> const T &
{
    return _data[i];
}

template<typename T>
inline auto core::pod::begin(Array<T> &a) noexcept -> T *
{
    return array::begin(a);
}

template<typename T>
inline auto core::pod::begin(const Array<T> &a) noexcept -> const T *
{
    return array::begin(a);
}

template<typename T>
inline auto core::pod::end(Array<T> &a) noexcept -> T *
{
    return array::end(a);
}

template<typename T>
inline auto core::pod::end(const Array<T> &a) noexcept -> const T *
{
    return array::end(a);
}

template<typename T>
inline void core::pod::swap(Array<T> &lhs, Array<T> &rhs) noexcept
{
    std::swap(lhs._allocator, rhs._allocator);
    std::swap(lhs._size, rhs._size);
    std::swap(lhs._capacity, rhs._capacity);
    std::swap(lhs._data, rhs._data);
}

// core::pod::Array free functions
//////////////////////////////////////////////////////////////////////////

template<typename T>
inline auto core::pod::array::size(const Array<T> &a) noexcept -> uint32_t
{
    return a._size;
}

template<typename T>
inline bool core::pod::array::any(const Array<T> &a) noexcept
{
    return a._size != 0;
}

template<typename T>
inline bool core::pod::array::empty(const Array<T> &a) noexcept
{
    return a._size == 0;
}

template<typename T>
inline auto core::pod::array::begin(Array<T> &a) noexcept -> T *
{
    return a._data;
}

template<typename T>
inline auto core::pod::array::begin(const Array<T> &a) noexcept -> const T *
{
    return a._data;
}

template<typename T>
inline auto core::pod::array::end(Array<T> &a) noexcept -> T *
{
    return a._data + a._size;
}

template<typename T>
inline auto core::pod::array::end(const Array<T> &a) noexcept -> const T *
{
    return a._data + a._size;
}

template<typename T>
inline auto core::pod::array::front(Array<T> &a) noexcept -> T &
{
    return a._data[0];
}

template<typename T>
inline auto core::pod::array::front(const Array<T> &a) noexcept -> const T &
{
    return a._data[0];
}

template<typename T>
inline auto core::pod::array::back(Array<T> &a) noexcept -> T &
{
    return a._data[a._size - 1];
}

template<typename T>
inline auto core::pod::array::back(const Array<T> &a) noexcept -> const T &
{
    return a._data[a._size - 1];
}

template<typename T>
inline void core::pod::array::clear(Array<T> &a) noexcept
{
    resize(a, 0);
}

template<typename T>
inline void core::pod::array::trim(Array<T> &a) noexcept
{
    set_capacity(a, a._size);
}

template<typename T>
inline void core::pod::array::resize(Array<T> &a, uint32_t new_size) noexcept
{
    if (new_size > a._capacity)
    {
        grow(a, new_size);
    }
    a._size = new_size;
}

template<typename T>
inline void core::pod::array::reserve(Array<T> &a, uint32_t new_capacity) noexcept
{
    if (new_capacity > a._capacity)
        set_capacity(a, new_capacity);
}

template<typename T>
inline void core::pod::array::set_capacity(Array<T> &a, uint32_t new_capacity) noexcept
{
    if (new_capacity == a._capacity)
    {
        return;
    }

    if (new_capacity < a._size)
    {
        resize(a, new_capacity);
    }

    T *new_data = 0;
    if (new_capacity > 0)
    {
        new_data = (T*)a._allocator->allocate(sizeof(T) * new_capacity, std::max(alignof(T), 4llu));
        memcpy(new_data, a._data, sizeof(T) * a._size);
    }

    a._allocator->deallocate(a._data);
    a._data = new_data;
    a._capacity = new_capacity;
}

template<typename T>
inline void core::pod::array::grow(Array<T> &a, uint32_t min_capacity) noexcept
{
    uint32_t new_capacity = a._capacity * 2 + 8;
    if (new_capacity < min_capacity)
    {
        new_capacity = min_capacity;
    }
    set_capacity(a, new_capacity);
}

template<typename T, typename U>
inline void core::pod::array::push_back(Array<T>& a, U const& item) noexcept
{
    //static_assert(std::is_convertible_v<U, T>, "Cannot push the given type 'U' to array of types 'T'");

    if (a._size + 1 > a._capacity)
    {
        grow(a);
    }
    a._data[a._size++] = item;
}

//! \brief Pushes all items to the end of the array.
template<typename T>
inline void core::pod::array::push_back(Array<T>& a, Array<T> const& items) noexcept
{
    if ((a._size + items._size) > a._capacity)
    {
        grow(a, a._size + items._size);
    }
    memcpy(a._data + a._size, items._data, items._size * sizeof(T));
    a._size += items._size;
}

template<typename T>
inline void core::pod::array::pop_back(Array<T> &a) noexcept
{
    a._size--;
}

template<typename T>
void core::pod::array::create_view(Array<T>& a, T* data, uint32_t count) noexcept
{
    a._data = data;
    a._size = count;
    a._capacity = count;
}

template<typename T>
auto core::pod::array::create_view(T* data, uint32_t count) noexcept -> core::pod::Array<T>
{
    Array<T> result{ core::memory::globals::null_allocator() };
    create_view<T>(result, data, count);
    return result;
}

template<typename T, uint32_t Size>
auto core::pod::array::create_view(T(&data)[Size]) noexcept -> Array<T>
{
    return create_view<std::decay_t<T>>(data, Size);
}