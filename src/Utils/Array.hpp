#ifndef CPPSYNTH_AUDIOSYSTEM_BUFFER_HPP
#define CPPSYNTH_AUDIOSYSTEM_BUFFER_HPP

#include <type_traits>

namespace CppSynth {

	/// <summary>
	/// Helper class that uses RAII to manage a heap-allocated array.
	/// </summary>
	template<typename T>
	class Array {
	public:
		// The proper type that the Buffer uses.
		// Can't think of a reason to remove const and pointers.
		using Type = std::remove_reference_t<T>;

		constexpr Array(size_t size = 0)
			: m_Size(size), m_Data(new Type[size])
		{ }

		constexpr Array(const Array& other)
		{
			m_Size = other.m_Size;
			m_Data = new T[other.m_Size];
			memcpy(m_Data, other.m_Data, other.m_Size * sizeof(T));
		}

		constexpr Array(Array&& other)
		{
			m_Size = other.m_Size;
			m_Data = other.m_Data;
			other.m_Data = nullptr;
		}

		constexpr ~Array()
		{
			delete[] m_Data;
		}

		constexpr Array& operator=(const Array& other) noexcept
		{
			m_Size = other.m_Size;
			m_Data = new T[other.m_Size];
			memcpy(m_Data, other.m_Data, other.m_Size * sizeof(T));
			return *this;
		}

		constexpr Array& operator=(Array&& other) noexcept
		{
			m_Size = other.m_Size;
			m_Data = other.m_Data;
			other.m_Data = nullptr;
			return *this;
		}

		inline constexpr Type* data() noexcept { return m_Data; }
		inline constexpr Type* operator*() noexcept { return *m_Data; }
		inline constexpr Type& operator[](size_t index) { return m_Data[index]; }
		inline constexpr Type* operator->() { return m_Data; }
		inline constexpr operator Type* () { return m_Data; }
		inline constexpr size_t size() noexcept { return m_Size; }

	private:
		Type* m_Data;
		size_t m_Size;
	};

}

#endif /* CPPSYNTH_AUDIOSYSTEM_BUFFER_HPP */
