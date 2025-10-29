export module toria.util:std;

import std;
import :basic_fixed_string;

template<class CharT, std::size_t N, class TTraits>
struct std::formatter<toria::util::basic_fixed_string<CharT, N, TTraits>>
	: public std::formatter<typename toria::util::basic_fixed_string<CharT,N,TTraits>::view_type>
{
	auto format(
		const toria::util::basic_fixed_string<CharT, N, TTraits>& str,
		std::format_context& ctx) const {
		return std::formatter<typename toria::util::basic_fixed_string<CharT,N, TTraits>::view_type>::format(str, ctx);
	}
};

template<class CharT, std::size_t N, class TTraits>
struct std::hash<toria::util::basic_fixed_string<CharT, N, TTraits>>
{
	std::size_t operator()(const toria::util::basic_fixed_string<CharT, N, TTraits> str) const {
		return std::hash<typename toria::util::basic_fixed_string<CharT, N, TTraits>::view_type>()(
			str);
	}
};
