import toria.uuid;
import std;
int main() {
	 using namespace std::literals;
	auto gen = toria::uuid::v5(toria::uuid::namespace_dns);
	auto gen2 = toria::uuid::v4();
	 auto a = gen("www.example.com");
	auto b = gen("test");
	std::cout << std::format("{}", a) << std ::endl;
	std::cout << std::format("{}", b) << std ::endl;
	std::cout << std::format("{}", gen2()) << std ::endl;
	return 0;
}
