export module toria.system:system_info;
import std;

namespace toria::system
{

	namespace system_info
	{
		export struct os_info
		{
			std::string name;
			std::string version;
			std::string build_number;
			std::string architecture;
			bool portable;
		};

		export os_info get_os_info() noexcept;

		export struct processor_info
		{

			std::string name;
			std::string socket;
			std::string architecture;
			std::string manufacturer;
			std::uint32_t cores;
			std::uint32_t enabled_cores;
			std::uint32_t logical_processors;
			std::uint16_t address_width;

		};

		export processor_info get_processor_info() noexcept;

		export struct gpu_info
		{
			std::string name;
			std::string driver_version;
			std::uint32_t vram;
		};

		export gpu_info get_gpu_info() noexcept;

		std::size_t page_size;
		std::size_t allocation_granularity;
	};
}
