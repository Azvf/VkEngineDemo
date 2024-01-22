#pragma once

namespace Chandelier{ 
	class NonCopyable {
	public:
		NonCopyable(const NonCopyable& other) = delete;
		NonCopyable& operator=(const NonCopyable& other) = delete;

		NonCopyable() = default;
		NonCopyable(NonCopyable&& other) = default;
		NonCopyable& operator=(NonCopyable&& other) = default;
	};

	class NonMovable {
	public:
		NonMovable(NonMovable&& other) = delete;
		NonMovable& operator=(NonMovable&& other) = delete;

		NonMovable() = default;
		NonMovable(const NonMovable& other) = default;
		NonMovable& operator=(const NonMovable& other) = default;
	};
}
