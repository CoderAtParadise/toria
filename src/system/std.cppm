export module toria.system:std;

import std;
import :thread;
import :process;

template<>
struct std::formatter<toria::system::thread::thread_priority>
{
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static auto
	format(const toria::system::thread::thread_priority& value, std::format_context& ctx) {
		switch (value) {
			case toria::system::thread::thread_priority::INACTIVE:
				return std::format_to(ctx.out(), "{}", "INACTIVE");
			case toria::system::thread::thread_priority::IDLE:
				return std::format_to(ctx.out(), "{}", "IDLE");
			case toria::system::thread::thread_priority::LOW:
				return std::format_to(ctx.out(), "{}", "LOW");
			case toria::system::thread::thread_priority::BELOW_NORMAL:
				return std::format_to(ctx.out(), "{}", "BELOW NORMAL");
			case toria::system::thread::thread_priority::ABOVE_NORMAL:
				return std::format_to(ctx.out(), "{}", "ABOVE NORMAL");
			case toria::system::thread::thread_priority::HIGH:
				return std::format_to(ctx.out(), "{}", "HIGH");
			case toria::system::thread::thread_priority::REALTIME:
				return std::format_to(ctx.out(), "{}", "REALTIME");
			case toria::system::thread::thread_priority::NORMAL:
			default:
				return std::format_to(ctx.out(), "{}", "NORMAL");
		}
	}
	// NOLINTEND(readability-convert-member-functions-to-static)
};

template<>
struct std::hash<toria::system::unique_pid_token>
{
	std::size_t operator()(const toria::system::unique_pid_token& token) const noexcept {
		using timepoint = std::chrono::time_point<std::chrono::file_clock>;
		const auto pid_hash = std::hash<std::uint64_t>{}(token.pid);
		const auto time_hash = std::hash<std::chrono::utc_clock::rep>{}(
			std::chrono::file_clock::to_utc(token.timestamp).time_since_epoch().count());
		return pid_hash ^ time_hash;
	}
};

 bool operator==(const toria::system::unique_pid_token& lhs, const std::uint64_t& rhs) noexcept {
	return  std::hash<toria::system::unique_pid_token>{}(lhs) == rhs;
}