#pragma once

#include <cerrno>
#include <cstdint>

/**
 * @brief Safely copies data from a source buffer to a destination variable and updates the offset.
 *
 * @tparam T Type of the destination variable.
 * @param[out] dest Reference to the destination variable where data will be copied.
 * @param[in,out] offset Reference to the current offset within the source buffer. This is updated after the copy.
 * @param[in] source Pointer to the source buffer from which data is copied.
 * @param[in] sourceSize The total size of the source buffer to ensure safe copying.
 * @return True if the copy is successful, false otherwise.
 */
template<typename T>
bool safeCopyFromBuffer(T& dest, size_t& offset, uint8_t const* source, size_t sourceSize)
{
	// Calculate remaining buffer size from current offset.
	size_t remainingBufferSize{ sourceSize - offset };

	// Ensure the variable does not exceed the remaining buffer size.
	if (sizeof(T) > remainingBufferSize)
	{
		std::cerr << __FUNCSIG__ << ": Copy exceeds buffer bounds\n";
		return false;
	}

	errno_t err{ memcpy_s(&dest, sizeof(T), source + offset, sizeof(T)) };
	if (err != 0)
	{
		std::cerr << __FUNCSIG__ << ": memcpy_s failed\n";
		return false;
	}

	// Update the offset.
	offset += sizeof(T);
	return true;
}
