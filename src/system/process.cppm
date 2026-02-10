export module toria.system:process;

import std;

namespace toria::system
{
	export namespace process
	{
		struct usage
		{
			std::chrono::time_point<std::chrono::file_clock> kernel_time{};
			std::chrono::time_point<std::chrono::file_clock> user_time{};
		};

		std::uint64_t get_current_id() noexcept;
		std::chrono::time_point<std::chrono::file_clock> get_creation_time() noexcept;
		usage get_usage_time() noexcept;
	};  // namespace process

	export struct unique_pid_token
	{
		static unique_pid_token get_unique_pid_token() noexcept  {
			return {process::get_current_id(), process::get_creation_time()};
		}

		static unique_pid_token get_unique_pid_token_for_process(std::uint64_t pid) noexcept;

		friend bool operator==(const unique_pid_token& lhs, const unique_pid_token& rhs) noexcept {
			return lhs.pid == rhs.pid && lhs.timestamp == rhs.timestamp;
		}

		std::uint64_t pid{0};
		std::chrono::time_point<std::chrono::file_clock> timestamp{};
	};

	constexpr unique_pid_token NULL{};
}  // namespace toria::system