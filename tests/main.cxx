
#ifdef __INTELLISENSE__
#include <cstddef>
#include <print>
#else
import toria.uuid;
import toria.crypto;
import toria.util;
import std;
#endif  // __INTELLISENSE__
using namespace std::literals;
constexpr toria::crypto::hash<toria::crypto::sha1> test() {
	toria::crypto::hash<toria::crypto::sha1> hash{};
	hash.hashString("teststring"sv);
	return hash;
}

int main() {
	constexpr auto gen3 = toria::uuid::v3(toria::uuid::namespace_dns);
	constexpr auto gen5 = toria::uuid::v5(toria::uuid::namespace_dns);
	auto gen4 = toria::uuid::v4();
	auto uuid3 = gen3("www.example.com");
	constexpr auto cuuid3 = gen3("www.example.com");
	auto uuid5 = gen5("www.example.com");
	constexpr auto cuuid5 = gen5("www.example.com");
	std::println("UUID V3 {}", uuid3);
	std::println("constexpr UUID V3 {}", cuuid3);
	std::println("UUID V5 {}", uuid5);
	std::println("constexpr UUID V5 {}", cuuid5);
	std::println("UUID V4 {}", gen4());
	return 0;
}
