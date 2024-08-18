#ifndef CPPSYNTH_AUDIOSYSTEM_BUFFER_HPP
#define CPPSYNTH_AUDIOSYSTEM_BUFFER_HPP

#include "Sample.hpp"
#include <type_traits>

namespace CppSynth {

	/// <summary>
	/// Helper class that uses RAII to manage a heap-allocated array.
	/// </summary>
	template<typename T>
	class Buffer {
	public:
		// The proper type that the Buffer uses.
		// Can't think of a reason to remove const and pointers.
		using Type = std::remove_reference_t<T>;

		constexpr Buffer(size_t size = 0)
			: m_Size(size), m_Data(new Type[size])
		{ }

		constexpr Buffer(const Buffer& other)
		{
			m_Size = other.m_Size;
			m_Data = new T[other.m_Size];
			memcpy(m_Data, other.m_Data, other.m_Size * sizeof(T));
		}

		constexpr Buffer(const Buffer&& other)
		{
			m_Size = other.m_Size;
			m_Data = other.m_Data;
			other.m_Data = nullptr;
		}

		constexpr ~Buffer()
		{
			delete[] m_Data;
		}

		constexpr Buffer& operator=(const Buffer& other) noexcept
		{
			m_Size = other.m_Size;
			m_Data = new T[other.m_Size];
			memcpy(m_Data, other.m_Data, other.m_Size * sizeof(T));
			return *this;
		}

		constexpr Buffer& operator=(const Buffer&& other) noexcept
		{
			m_Size = other.m_Size;
			m_Data = other.m_Data;
			other.m_Data = nullptr;
			return *this;
		}

		inline constexpr Type* Data() noexcept { return m_Data; }
		inline constexpr Type* operator*() noexcept { return *m_Data; }
		inline constexpr Type& operator[](size_t index) { return m_Data[index]; }
		inline constexpr Type* operator->() { return m_Data; }
		inline constexpr operator Type* () { return m_Data; }
		inline constexpr size_t Size() noexcept { return m_Size; }

	private:
		Type* m_Data;
		size_t m_Size;
	};

	/// <summary>
	/// A buffer of samples. "A" stands for "Audio.
	/// </summary>
	using ABuffer = Buffer<Sample>;

}

#endif /* CPPSYNTH_AUDIOSYSTEM_BUFFER_HPP */