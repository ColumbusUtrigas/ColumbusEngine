#pragma once

#include <algorithm>
#include <array>
#include <type_traits>
#include <cassert>

#define _fixed_vector_type_check(otherType) static_assert(std::is_same<otherType, T>::value, "fixed_vector types are incopatible");
#define _fixed_vector_static_check(otherType, otherSize) \
	static_assert(otherSize <= Size, "fixed_vector size error"); \
	_fixed_vector_type_check(otherType);

template <typename T, size_t Size>
struct fixed_vector
{
private:
	std::array<T, Size> _internal;
	size_t _used = 0;
public:
	template <typename _PointerType, typename _ReferenceType>
	struct generic_iterator
	{
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
		using difference_type = ptrdiff_t;
		using pointer = _PointerType;
		using reference = _ReferenceType;

		explicit generic_iterator(pointer beginning) : _ptr(beginning) {}
		generic_iterator& operator++() { ++_ptr; return *this; }
		generic_iterator operator++(int) { generic_iterator retval = *this; ++(*this); return retval; }
		bool operator==(generic_iterator other) const { return _ptr == other._ptr; }
		bool operator!=(generic_iterator other) const { return !(*this == other); }
		reference operator*() const { return *_ptr; }

	protected:
		pointer _ptr;
	};

	using iterator = generic_iterator<T*, T&>;
	using const_iterator = generic_iterator<const T*, const T&>;
public:
	fixed_vector() {}

	template <typename CT, size_t CSize>
	fixed_vector(const fixed_vector<CT, CSize>& base)
	{
		_fixed_vector_static_check(CT, CSize);
		*this = base;
	}

	template <typename CT, size_t CSize>
	fixed_vector(fixed_vector<CT, CSize>&& base)
	{
		_fixed_vector_static_check(CT, CSize);
		*this = std::move(base);
	}

	template <typename CT>
	fixed_vector(std::initializer_list<CT> base)
	{
		assert(Size > base.size());
		*this = base;
	}

	template <typename CT, size_t CSize>
	fixed_vector& operator=(const fixed_vector<CT, CSize>& other)
	{
		_fixed_vector_static_check(CT, CSize);
		_internal = other._internal;
		_used = other._used;
		return *this;
	}

	template <typename CT, size_t CSize>
	fixed_vector& operator=(fixed_vector<CT, CSize>&& other)
	{
		_fixed_vector_static_check(CT, CSize);
		_internal = std::move(other._internal);
		_used = std::move(other._used);
		return *this;
	}

	template <typename CT>
	fixed_vector& operator=(std::initializer_list<CT> other)
	{
		_fixed_vector_type_check(CT);
		assert(Size > other.size());
		_used = 0;

		std::copy_if(other.begin(), other.end(), _internal.begin(), [&](auto& x) {
			_used++;
			return true;
		});
		return *this;
	}

	fixed_vector& push_back(const T& value)
	{
		_internal[_used++] = value;
		return *this;
	}

	size_t size() const noexcept { return _used; }
	constexpr size_t capacity() const noexcept{ return Size; }

	T& operator[](size_t id)
	{
		assert(id < _used);
		return _internal[id];
	}

	const T& operator[](size_t id) const
	{
		assert(id < _used);
		return _internal[id];
	}

	iterator begin() { return iterator(_internal.data()); }
	iterator end() { return iterator(_internal.data() + _used); }
	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }

	const_iterator cbegin() const { return const_iterator(_internal.data()); }
	const_iterator cend() const { return const_iterator(_internal.data() + _used); }
};

#undef _fixed_vector_static_check
#undef _fixed_vector_type_check
