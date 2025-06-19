export module toria.util:tracing_exception;

#ifdef __INTELLISENSE__
#include <stacktrace>
#else
import std;
#endif  // __INTELLISENSE__

namespace toria
{
	namespace utils
	{
		class tracing_exception
		{
			constexpr tracing_exception(std::stacktrace stacktrace = std::stacktrace::current())
				: m_stacktrace(stacktrace) {};
			constexpr virtual ~tracing_exception() = default;
			constexpr tracing_exception(const tracing_exception& rhs) noexcept;
			constexpr virtual const char* what() const noexcept = 0;

		private:
			std::stacktrace m_stacktrace;
		};
	}  // namespace utils
}  // namespace toria
