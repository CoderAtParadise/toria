export module toria.system:thread;

import std;

namespace toria::system
{
	export namespace thread
	{
		enum class thread_priority
		{
			INACTIVE, // Returned when a thread doesn't exist
			IDLE,
			LOW,
			BELOW_NORMAL,
			NORMAL,
			ABOVE_NORMAL,
			HIGH,
			REALTIME,
		};

		std::string get_thread_name(std::thread::id threadId) noexcept;
		bool set_thread_name(std::thread::id threadID, std::string_view name) noexcept;
		thread_priority get_thread_priority(std::thread::id threadId) noexcept;
		bool set_thread_priority(std::thread::id threadID,thread_priority priority) noexcept;
		bool is_background_thread(std::thread::id threadID) noexcept;
		/**
		 * Switches a thread into background mode
		 * @return If the thread successfully switched mode
		 */
		bool switch_thread_background_mode(std::thread::id threadId,bool setBackground) noexcept;
	}  // namespace thread
}  // namespace toria::system