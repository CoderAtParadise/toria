export module toria.semver:impl;

import std;
import :identifier;

namespace toria::semver
{
	template<identifier_storage Storage>
	class [[nodiscard]] version_impl : Storage
	{
	public:
		constexpr version_impl(
			const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch,
			Storage identifier_storage = {}) noexcept
			: Storage(std::move(identifier_storage)), m_major(major), m_minor(minor),
			  m_patch(patch) {
		}

		explicit operator version_impl<dynamic_identifiers>() const noexcept requires (!
			std::same_as<Storage, dynamic_identifiers>) {
			return {
				major(), minor(), patch(), dynamic_identifiers(static_cast<Storage>(*this))
			};
		}

		[[nodiscard]] constexpr std::uint32_t major() const noexcept { return m_major; }
		[[nodiscard]] constexpr std::uint32_t minor() const noexcept { return m_minor; }
		[[nodiscard]] constexpr std::uint32_t patch() const noexcept { return m_patch; }

		template<
			bool UsePrerelease = true, identifier_storage LStorage,
			identifier_storage RStorage>
		constexpr friend bool
		operator==(const version_impl<LStorage>& lhs, const version_impl<RStorage>& rhs) {
			if (lhs.major() != rhs.major() || lhs.minor() != rhs.minor() ||
			    lhs.patch() != rhs.patch())
				return false;
			if (!UsePrerelease)
				return true;
			return lhs.prerelease_tag() <=> rhs.prerelease_tag();
		}

		template<
			bool UsePrerelease = true, identifier_storage LStorage,
			identifier_storage RStorage>
		constexpr friend std::strong_ordering
		operator<=>(const version_impl<LStorage>& lhs, const version_impl<RStorage>& rhs) {
			const auto majorCmp = lhs.major() <=> rhs.major();
			if (majorCmp != std::strong_ordering::equal)
				return majorCmp;
			const auto minorCmp = lhs.minor() <=> rhs.minor();
			if (minorCmp != std::strong_ordering::equal)
				return minorCmp;
			const auto patchCmp = lhs.patch() <=> rhs.patch();
			if (!UsePrerelease || patchCmp != std::strong_ordering::equal) {
				return patchCmp;
			}
			return lhs.prerelease_tag() <=> rhs.prerelease_tag();
		}

	private:
		std::uint32_t m_major{0};
		std::uint32_t m_minor{0};
		std::uint32_t m_patch{0};
	};
} // namespace toria::semver
