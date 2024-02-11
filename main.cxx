import toria;
import std;
int main() {
	 using namespace std::literals;
	auto gen = toria::v4();
	auto a = gen();
	auto b = gen();
	std::cout << std::format("{}", a) << std ::endl;
	std::cout << std::format("{}", b) << std ::endl;
	return 0;
}
