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

		static auto get_current_id() noexcept -> std::uint64_t;
		static auto get_creation_time() noexcept
			-> std::chrono::time_point<std::chrono::file_clock>;
		static auto get_usage_time() noexcept -> usage;
	};

	export struct unique_pid_token
	{
		static auto get_unique_pid_token() noexcept -> unique_pid_token {
			return {process::get_current_id(), process::get_creation_time()};
		}

		static auto get_unique_pid_token_for_process(std::uint64_t pid) noexcept
			-> unique_pid_token;

		friend auto operator==(const unique_pid_token& lhs, const unique_pid_token& rhs) noexcept
			-> bool {
			return lhs.pid == rhs.pid && lhs.timestamp == rhs.timestamp;
		}

		std::uint64_t pid{};
		std::chrono::time_point<std::chrono::file_clock> timestamp{};
	};

}  // namespace toria::system