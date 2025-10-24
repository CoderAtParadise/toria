import toria.uuid;
import toria.crypto;
import toria.util;
import std;

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
	return 0;
	// 3e a6 8d 6e  2a 08 0b 85
}
