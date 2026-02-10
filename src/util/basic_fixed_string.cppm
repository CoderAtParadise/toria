export module toria.util:basic_fixed_string;

import std;

namespace toria::util
{
	export template<class CharT, std::size_t N, class Traits = std::char_traits<CharT>>
	class basic_fixed_string : public std::array<CharT, N>
	{
		using super = std::array<CharT, N>;

	public:
		using traits_type = Traits;
		using iterator = super::const_iterator;
		using const_reverse_iterator = std::reverse_iterator<typename super::const_iterator>;
		using reverse_iterator = const_reverse_iterator;
		using string_view_type = std::basic_string_view<typename super::value_type, traits_type>;

		static constexpr auto npos = string_view_type::npos;

		template<std::size_t... Idx>
		consteval basic_fixed_string(const char* in, std::index_sequence<Idx...>) noexcept
			: super{in[Idx]...} {}

		// NOLINTNEXTLINE(google-explicit-constructor)
		consteval basic_fixed_string(const CharT (&in)[N + 1]) noexcept
			: super{basic_fixed_string(in, std::make_index_sequence<N>())} {}

		// NOLINTNEXTLINE(google-explicit-constructor)
		consteval basic_fixed_string(const std::span<const super::value_type, N> in) noexcept
			: super{in} {}

		[[nodiscard]] constexpr super::size_type length() const noexcept { return this->size(); }

		[[nodiscard]] constexpr super::const_pointer c_str() const noexcept { return this->data(); }

		// NOLINTNEXTLINE(google-explicit-constructor)
		constexpr operator string_view_type() const noexcept {
			return {this->data(), this->size()};
		}

		[[nodiscard]] constexpr iterator begin() noexcept { return this->begin(); }
		[[nodiscard]] constexpr iterator end() noexcept { return this->end(); }
		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return this->rbegin(); }
		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return this->rend(); }

		constexpr super::size_type
		copy(CharT* dest, super::size_type count, super::size_type pos = 0) const
			requires(pos <= super::size())
		{
			const typename super::size_type rcount = std::min(count, super::size() - pos);
			Traits::copy(dest, this->data() + pos, rcount);
			return rcount;
		}

		template<typename super::size_type Off, typename super::size_type Count = npos>
			requires(Off < super::size())
		constexpr auto substr() const noexcept {
			constexpr typename super::size_type new_size = std::min(Count, this->size() - Off);
			return basic_fixed_string<CharT, new_size>({this->data() + Off, new_size});
		}

		constexpr auto substr(super::size_type Off, super::size_type Count = npos) const noexcept {
			return view().substr(Off, Count);
		}

		template<typename super::size_type M>
		[[nodiscard]] friend constexpr bool
		operator==(const basic_fixed_string& lhs, const basic_fixed_string<CharT, M, Traits>& rhs) {
			return lhs.view() == rhs.view();
		}

		[[nodiscard]] friend constexpr bool
		operator==(const basic_fixed_string& lhs, std::basic_string_view<CharT, Traits> rhs) {
			return lhs.view() == rhs;
		}

		[[nodiscard]] friend constexpr bool operator==(
			const std::basic_string_view<CharT, Traits>& lhs, const basic_fixed_string& rhs) {
			return lhs == rhs.view();
		}

		template<typename super::size_type M>
		[[nodiscard]] friend constexpr std::strong_ordering operator<=>(
			const basic_fixed_string& lhs, const basic_fixed_string<CharT, M, Traits>& rhs) {
			return lhs.view() <=> rhs.view();
		}

		[[nodiscard]] constexpr super::const_reference operator[](super::size_type pos) const
			requires(pos < this->size())
		{
			return super::operator[](pos);
		}

		[[nodiscard]] constexpr super::const_reference at(super::size_type pos) const
			requires(pos < this -> size())
		{
			return this->at(pos);
		}

		[[nodiscard]] friend constexpr std::strong_ordering
		operator<=>(const basic_fixed_string& lhs, std::basic_string_view<CharT, Traits> rhs) {
			return lhs.view() <=> rhs;
		}

		[[nodiscard]] friend constexpr std::strong_ordering
		operator<=>(std::basic_string_view<CharT, Traits> lhs, const basic_fixed_string& rhs) {
			return lhs <=> rhs.view();
		}

		template<typename super::size_type M>
		[[nodiscard]] constexpr super::size_type find(
			const basic_fixed_string<CharT, M, Traits>& str,
			super::size_type pos = 0) const noexcept {
			if constexpr (M > this->size())
				return npos;
			return view().find(str.view(), pos);
		}
		[[nodiscard]] constexpr super::size_type
		find(string_view_type view, super::size_type pos = 0) noexcept {
			return this->view().find(view, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find(const CharT* s, super::size_type pos, super::size_type n) const noexcept {
			return view().find(s, pos, n);
		}
		[[nodiscard]] constexpr super::size_type
		find(const CharT* s, super::size_type pos = 0) const noexcept {
			return view().find(s, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find(CharT ch, super::size_type pos = 0) const noexcept {
			return view().find(ch, pos);
		}

		template<typename super::size_type M>
		[[nodiscard]] constexpr super::size_type rfind(
			const basic_fixed_string<CharT, M, Traits>& str,
			super::size_type pos = npos) const noexcept {
			if constexpr (M > this->size())
				return npos;
			return view().rfind(str.view(), pos);
		}
		[[nodiscard]] constexpr super::size_type
		rfind(string_view_type view, super::size_type pos = npos) const noexcept {
			return this->view().rfind(view, pos);
		}
		[[nodiscard]] constexpr super::size_type
		rfind(const CharT* s, super::size_type pos, super::size_type n) const noexcept {
			return view().rfind(s, pos, n);
		}
		[[nodiscard]] constexpr super::size_type
		rfind(const CharT* s, super::size_type pos = npos) const noexcept {
			return view().rfind(s, pos);
		}
		[[nodiscard]] constexpr super::size_type
		rfind(CharT ch, super::size_type pos = npos) const noexcept {
			return view().rfind(ch, pos);
		}

		template<typename super::size_type M>
		[[nodiscard]] constexpr super::size_type find_first_of(
			const basic_fixed_string<CharT, M, Traits>& str,
			super::size_type pos = 0) const noexcept {
			if constexpr (M > this->size())
				return npos;
			return view().find_first_of(str.view(), pos);
		}
		[[nodiscard]] constexpr auto
		find_first_of(string_view_type view, super::size_type pos = 0) const noexcept
			-> super::size_type {
			return this->view().find_first_of(view, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_first_of(const CharT* s, super::size_type pos, super::size_type n) const noexcept {
			return view().find_first_of(s, pos, n);
		}
		[[nodiscard]] constexpr super::size_type
		find_first_of(const CharT* s, super::size_type pos = 0) const noexcept {
			return view().find_first_of(s, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_first_of(CharT ch, super::size_type pos = 0) const noexcept {
			return view().find_first_of(ch, pos);
		}

		template<typename super::size_type M>
		[[nodiscard]] constexpr super::size_type find_last_of(
			const basic_fixed_string<CharT, M, Traits>& str,
			super::size_type pos = npos) const noexcept {
			if constexpr (M > this->size())
				return npos;
			return view().find_last_of(str.view(), pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_last_of(string_view_type view, super::size_type pos = npos) const noexcept {
			return this->view().find_last_of(view, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_last_of(const CharT* s, super::size_type pos, super::size_type n) const noexcept {
			return view().find_last_of(s, pos, n);
		}
		[[nodiscard]] constexpr super::size_type
		find_last_of(const CharT* s, super::size_type pos = npos) const noexcept {
			return view().find_last_of(s, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_last_of(CharT ch, super::size_type pos = npos) const noexcept {
			return view().find_last_of(ch, pos);
		}

		template<typename super::size_type M>
		[[nodiscard]] constexpr super::size_type find_first_not_of(
			const basic_fixed_string<CharT, M, Traits>& str,
			super::size_type pos = 0) const noexcept {
			if constexpr (M > this->size())
				return npos;
			return view().find_first_not_of(str.view(), pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_first_not_of(string_view_type view, super::size_type pos = 0) const noexcept {
			return this->view().find_first_not_of(view, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_first_not_of(const CharT* s, super::size_type pos, super::size_type n) const noexcept {
			return view().find_first_not_of(s, pos, n);
		}
		[[nodiscard]] constexpr super::size_type
		find_first_not_of(const CharT* s, super::size_type pos = 0) const noexcept {
			return view().find_first_not_of(s, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_first_not_of(CharT ch, super::size_type pos = 0) const noexcept {
			return view().find_first_not_of(ch, pos);
		}

		template<typename super::size_type M>
		[[nodiscard]] constexpr super::size_type find_last_not_of(
			const basic_fixed_string<CharT, M, Traits>& str,
			super::size_type pos = npos) const noexcept {
			if constexpr (M > this->size())
				return npos;
			return view().find_last_not_of(str.view(), pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_last_not_of(string_view_type view, super::size_type pos = npos) const noexcept {
			return this->view().find_last_not_of(view, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_last_not_of(const CharT* s, super::size_type pos, super::size_type n) const noexcept {
			return view().find_last_not_of(s, pos, n);
		}
		[[nodiscard]] constexpr super::size_type
		find_last_not_of(const CharT* s, super::size_type pos = npos) const noexcept {
			return view().find_last_not_of(s, pos);
		}
		[[nodiscard]] constexpr super::size_type
		find_last_not_of(CharT ch, super::size_type pos = npos) const noexcept {
			return view().find_last_not_of(ch, pos);
		}

		[[nodiscard]] constexpr int compare(string_view_type view) const noexcept {
			return this->view().compar(view);
		}
		[[nodiscard]] constexpr int
		compare(super::size_type pos, super::size_type count, string_view_type view) const {
			return this->view().compare(pos, count, view);
		}
		[[nodiscard]] constexpr int compare(
			super::size_type pos1, super::size_type count1, string_view_type view,
			super::size_type pos2, super::size_type count2) {
			return this->view().compare(pos1, count1, view, pos2, count2);
		}

		[[nodiscard]] constexpr int compare(const CharT* s) const { return view().compare(s); }

		[[nodiscard]] constexpr int
		compare(super::size_type pos, super::size_type count, const CharT* s) const {
			return view().compare(pos, count, s);
		}

		[[nodiscard]] constexpr int compare(
			super::size_type pos1, super::size_type count1, const CharT* s,
			super::size_type count2) const {
			return view().compare(pos1, count1, s, count2);
		}

		[[nodiscard]] constexpr bool starts_with(string_view_type view) const noexcept {
			return this->view().starts_with(view);
		}
		[[nodiscard]] constexpr bool starts_with(CharT ch) const noexcept {
			return view().starts_with(ch);
		}
		[[nodiscard]] constexpr bool starts_with(const CharT* s) const {
			return view().starts_with(s);
		}
		[[nodiscard]] constexpr bool ends_with(string_view_type view) const noexcept {
			return this->view().ends_with(view);
		}
		[[nodiscard]] constexpr bool ends_with(CharT ch) const noexcept {
			return view().ends_with(ch);
		}
		[[nodiscard]] constexpr bool ends_with(const CharT* s) const { return view().ends_with(s); }

		[[nodiscard]] constexpr bool contains(string_view_type view) const noexcept {
			return this->view().contains(view);
		}
		[[nodiscard]] constexpr bool contains(CharT ch) const noexcept {
			return view().contains(ch);
		}
		[[nodiscard]] constexpr bool contains(const CharT* s) const { return view().contains(s); }

		[[nodiscard]] constexpr string_view_type view() const noexcept { return *this; }
	};

	template<class CharT>
	class basic_fixed_string<CharT, 0, std::char_traits<CharT>>
	{
		using super = std::array<CharT, 1>;
		using string_view_type = std::basic_string_view<CharT>;

	public:
		static constexpr super::size_type npos = string_view_type::npos;

		// NOLINTNEXTLINE(google-explicit-constructor)
		consteval basic_fixed_string(const CharT*) noexcept {}

		[[nodiscard]] static constexpr super::size_type size() noexcept { return 0; }

		[[nodiscard]] static consteval bool empty() noexcept { return true; }

		[[nodiscard]] consteval operator string_view_type() const noexcept { return ""; }
	};

	template<class CharT, std::size_t N>
	basic_fixed_string(const CharT (&)[N]) -> basic_fixed_string<CharT, N - 1>;

	export template<std::size_t N>
	using fixed_string = basic_fixed_string<char, N>;
	export template<std::size_t N>
	using fixed_wstring = basic_fixed_string<wchar_t, N>;
	export template<std::size_t N>
	using fixed_u8string = basic_fixed_string<char8_t, N>;
	export template<std::size_t N>
	using fixed_u16string = basic_fixed_string<char16_t, N>;
	export template<std::size_t N>
	using fixed_u32string = basic_fixed_string<char32_t, N>;
}  // namespace toria::util
