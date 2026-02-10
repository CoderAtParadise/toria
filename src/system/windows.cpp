module;
#include <windows.h>
#include <Processthreadsapi.h>
module toria.system;

namespace toria::system
{
	std::uint64_t process::get_current_id() noexcept {
		static std::uint64_t current_process_id = []() -> std::uint64_t {
			const DWORD pid = GetCurrentProcessId();
			return pid;
		}();
		return current_process_id;
	}

	static std::uint64_t fromFILETIME(const FILETIME* ft) noexcept {
		return static_cast<std::uint64_t>(ft->dwHighDateTime) << 32 | ft->dwLowDateTime;
	}

	std::chrono::time_point<std::chrono::file_clock> process::get_creation_time() noexcept {
		using time_point = std::chrono::time_point<std::chrono::file_clock>;
		static auto creation_time = []() -> time_point {
			if (const auto hProcess = GetCurrentProcess(); hProcess != nullptr) {
				FILETIME creationTime{}, dump{};
				if (GetProcessTimes(hProcess, &creationTime, &dump, &dump, &dump)) {
					return time_point{time_point::duration(fromFILETIME(&creationTime))};
				}
			}
			return {};
		}();
		return creation_time;
	}

	process::usage process::get_usage_time() noexcept {
		using time_point = std::chrono::time_point<std::chrono::file_clock>;
		if (const auto hProcess = GetCurrentProcess(); hProcess != nullptr) {
			FILETIME kernelTime, userTime, dump;
			if (GetProcessTimes(hProcess, &dump, &dump, &kernelTime, &userTime)) {
				return {
					time_point{time_point::duration(fromFILETIME(&kernelTime))},
					time_point{time_point::duration(fromFILETIME(&userTime))}};
			}
		}
		return {};
	}

	unique_pid_token
	unique_pid_token::get_unique_pid_token_for_process(std::uint64_t pid) noexcept {
		using time_point = std::chrono::time_point<std::chrono::file_clock>;
		if (const auto hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);
			hProcess != nullptr) {
			FILETIME creationTime, dump;
			if (GetProcessTimes(hProcess, &creationTime, &dump, &dump, &dump)) {
				return {pid, time_point{time_point::duration(fromFILETIME(&creationTime))}};
			}
		}
		return {-1ull, {}};
	}

	std::string thread::get_thread_name(const std::thread::id threadId) noexcept {
		const auto underlying = threadId._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, underlying);
		if (hThread == nullptr)
			return "";
		PWSTR data = nullptr;
		if (const auto hr = GetThreadDescription(hThread, &data); SUCCEEDED(hr)) {
			if (const int count =
					WideCharToMultiByte(CP_UTF8, 0, data, -1, nullptr, 0, nullptr, nullptr);
				count > 1 && data[0] != '\0') {
				auto str = std::string(count - 1, 0);  // Set size minus null termination
				WideCharToMultiByte(CP_UTF8, 0, data, -1, &str[0], count, nullptr, nullptr);
				LocalFree(data);
				return str;
			}
			LocalFree(data);
		}
		return std::format("{}", threadId);
	}

	bool
	thread::set_thread_name(const std::thread::id threadID, const std::string_view name) noexcept {
		const auto underlying = threadID._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, underlying);
		if (hThread == nullptr)
			return false;
		const int count =
			MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
		auto wName = std::wstring(count + 10, 0);
		MultiByteToWideChar(
			CP_UTF8, 0, &name[0], static_cast<int>(name.size()), wName.data(), count);
		return SUCCEEDED(SetThreadDescription(hThread, wName.c_str()));
	}

	static thread::thread_priority _map_native_to_priority(HANDLE thread, const int priority) {
		switch (priority) {
			case THREAD_PRIORITY_IDLE:
				return thread::thread_priority::IDLE;
			case THREAD_PRIORITY_LOWEST:
				return thread::thread_priority::LOW;
			case THREAD_PRIORITY_BELOW_NORMAL:
				return thread::thread_priority::BELOW_NORMAL;
			case THREAD_PRIORITY_ABOVE_NORMAL:
				return thread::thread_priority::ABOVE_NORMAL;
			case THREAD_PRIORITY_HIGHEST:
				return thread::thread_priority::HIGH;
			case THREAD_PRIORITY_TIME_CRITICAL:
				return thread::thread_priority::REALTIME;
			case THREAD_PRIORITY_NORMAL:
			default:
				return thread::thread_priority::NORMAL;
		}
	}

	static int _map_priority_to_native(const thread::thread_priority priority) noexcept {
		switch (priority) {
			case thread::thread_priority::IDLE:
				return THREAD_PRIORITY_IDLE;
			case thread::thread_priority::LOW:
				return THREAD_PRIORITY_LOWEST;
			case thread::thread_priority::BELOW_NORMAL:
				return THREAD_PRIORITY_BELOW_NORMAL;
			case thread::thread_priority::ABOVE_NORMAL:
				return THREAD_PRIORITY_ABOVE_NORMAL;
			case thread::thread_priority::HIGH:
				return THREAD_PRIORITY_HIGHEST;
			case thread::thread_priority::REALTIME:
				return THREAD_PRIORITY_TIME_CRITICAL;
			case thread::thread_priority::NORMAL:
			default:
				return THREAD_PRIORITY_NORMAL;
		}
	}

	thread::thread_priority thread::get_thread_priority(const std::thread::id id) noexcept {
		const auto underlying = id._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, underlying);
		if (!hThread)
			return thread_priority::INACTIVE;
		const int priority = GetThreadPriority(hThread);
		return _map_native_to_priority(hThread, priority);
	}

	bool thread::set_thread_priority(const std::thread::id id, thread_priority priority) noexcept {
		const auto underlying = id._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, underlying);
		if (!hThread)
			return false;
		return SUCCEEDED(SetThreadPriority(hThread, _map_priority_to_native(priority)));
	}

	bool is_background_thread(const std::thread::id id) noexcept {
		const auto underlying = id._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, underlying);
		if (!hThread)
			return false;
		const auto setToBackground =
			SUCCEEDED(SetThreadPriority(hThread, THREAD_MODE_BACKGROUND_BEGIN));
		if (setToBackground) {
			SetThreadPriority(hThread, THREAD_MODE_BACKGROUND_END);
		}
		return !setToBackground;
	}

	bool switch_thread_mode(const std::thread::id id, const bool setBackground) noexcept {
		const auto underlying = id._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, underlying);
		if (!hThread)
			return false;
		if (setBackground)
			return SUCCEEDED(SetThreadPriority(hThread, THREAD_MODE_BACKGROUND_BEGIN));
		return SUCCEEDED(SetThreadPriority(hThread, THREAD_MODE_BACKGROUND_END));
	}

}  // namespace toria::system