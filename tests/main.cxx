
#ifdef __INTELLISENSE__
#include <cstddef>
#include <span>
#include <print>
#include <iostream>
#include <sstream>
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

struct testing
{
	int x;
	int y;
	int z;
};

int main() {
	using namespace std::literals;
	constexpr auto gen = toria::uuid::v5(toria::uuid::namespace_dns);
	std::string_view xz = "www.example.com";
	std::span<const char> xa{xz};
	auto gen2 = toria::uuid::v4();
	constexpr auto gen3 = toria::uuid::v3(toria::uuid::namespace_dns);
	constexpr auto a = gen("www.example.com");
	constexpr auto b = gen3("www.example.com");
	constexpr auto t = test();
	constexpr auto d = toria::util::as_bytes(testing{16, 32, 168});
	constexpr testing e = toria::util::from_bytes<testing>(d.bytes());
	std::cout << std::format("{}", b) << std ::endl;
	std::cout << std::format("{}", a) << std ::endl;
	std::cout << std::format("{}", gen2()) << std ::endl;
	std::cout << std::format("{}", t) << std ::endl;
	return 0;
}
