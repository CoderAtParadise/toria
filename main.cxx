import toria;
import std;
int main() {
	 using namespace std::literals;
	auto gen = toria::v5(toria::namespace_dns);
	 auto a = gen("www.example.com");
	auto b = gen("test");
	std::cout << std::format("{}", a) << std ::endl;
	std::cout << std::format("{}", b) << std ::endl;
	return 0;
}
