
#ifdef __INTELLISENSE__
#include <cstddef>
#include <span>
#include <print>
#include <iostream>
#include <sstream>
#else
import toria.uuid;
import toria.crypto;
import std;
#endif  // __INTELLISENSE__
using namespace std::literals;
constexpr toria::crypto::hash<toria::crypto::sha1> test() {
	toria::crypto::hash<toria::crypto::sha1> hash{};
	hash.hashString("teststring"sv);
	return hash;
}

int main() {
	using namespace std::literals;
	constexpr auto gen = toria::uuid::v5(toria::uuid::namespace_dns);
	auto gen2 = toria::uuid::v4();
	// auto gen3 = toria::uuid::v3(toria::uuid::namespace_dns);
	constexpr auto a = gen("www.example.com");
	auto b = gen("www.example.com");
	std::cout << std::format("{}", a) << std ::endl;
	std::cout << std::format("{}", b) << std ::endl;
	std::cout << std::format("{}", gen2()) << std ::endl;
	constexpr auto t1 = test();
	std::cout << std::format("{}", t1);
	return 0;
}
