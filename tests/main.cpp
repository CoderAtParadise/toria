import std;
import toria.uuid;
import toria.crypto;
import toria.util;
import toria.system;
import std;
import toria.semver;

int main() {
	constexpr auto gen3 = toria::uuid::v3(toria::uuid::namespace_dns);
	constexpr auto gen5 = toria::uuid::v5(toria::uuid::namespace_dns);
	auto gen4 = toria::uuid::v4();
	auto uuid3 = gen3("www.example.com");
	constexpr auto cuuid3 = gen3("www.example.com");
	auto uuid5 = gen5("www.example.com");
	constexpr auto cuuid5 = gen5("www.example.com");
	auto gen7 = toria::uuid::v7();
	std::println("UUID V3 {}", uuid3);
	std::println("constexpr UUID V3 {}", cuuid3);
	std::println("UUID V5 {}", uuid5);
	std::println("constexpr UUID V5 {}", cuuid5);
	std::println("UUID V4 {}", gen4());
	std::println("UUID V7 {}", gen7());
	std::println("UUID v7 2 {}", gen7());
	constexpr auto sem = toria::semver::semver_impl_version;
	constexpr auto sem2 = toria::semver::version<"test.hello.">(1, 1, 1);
	/*std::println("{}", toria::system::thread::get_thread_name(std::this_thread::get_id()));
	toria::system::thread::set_thread_name(std::this_thread::get_id(), "MainThread");
	std::println("{}", toria::system::thread::get_thread_name(std::this_thread::get_id()));
	toria::system::thread::set_thread_name(std::this_thread::get_id(), "RenderThread");
	std::println("{}", toria::system::thread::get_thread_name(std::this_thread::get_id()));
	std::println("{}", toria::system::thread::get_thread_priority(std::this_thread::get_id()).value());
	std::println("{}", std::chrono::system_clock::now());
	std::println(
		"{}", toria::system::process::get_creation_time());

	std::println("OS: {}",toria::system::system_info::get_os_info().name);
	std::println("{}",toria::system::system_info::get_processor_info().architecture);
	std::println("{}",toria::system::system_info::get_gpu_info().name);
	try {
		auto res = toria::semver::try_parse("2.1");
	} catch (const parse_exception& err) {
		std::println("{}", err.what());
	}
	constexpr auto res2 = toria::semver::version<"2.1.21-world+ewwo.night">();
	return 0;
}
