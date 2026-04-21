export module toria.system:thread;

import std;

namespace toria::system
{
    export namespace thread
    {
        enum class thread_priority
        {
            IDLE,
            LOW,
            BELOW_NORMAL,
            NORMAL,
            ABOVE_NORMAL,
            HIGH,
            REALTIME,
        };

        std::string get_thread_name(std::thread& thread) noexcept;
    	std::string get_thread_name(std::thread::id threadId) noexcept;
        bool set_thread_name(std::thread& thread, std::string_view name) noexcept;
        bool set_thread_name(std::thread::id threadID, std::string_view name) noexcept;

        std::optional<thread_priority> get_thread_priority(std::thread& thread) noexcept;
        std::optional<thread_priority> get_thread_priority(std::thread::id threadId) noexcept;
        bool set_thread_priority(std::thread& thread, thread_priority priority) noexcept;
        bool set_thread_priority(std::thread::id threadID, thread_priority priority) noexcept;

        bool is_background_thread(std::thread& thread) noexcept;
        bool is_background_thread(std::thread::id threadID) noexcept;
        bool switch_thread_background_mode(std::thread& thread, bool setBackground) noexcept;
        bool switch_thread_background_mode(std::thread::id threadId, bool setBackground) noexcept;

    	/*std::uint32_t get_thread_affinity(std::thread& thread) noexcept;
    	std::uint32_t get_thread_affinity(std::thread::id threadId) noexcept;
        void set_thread_affinity(std::thread& thread, std::uint32_t core) noexcept;
        void set_thread_affinity(std::thread::id threadId, std::uint32_t core) noexcept; */
    } // namespace thread
} // namespace toria::system
