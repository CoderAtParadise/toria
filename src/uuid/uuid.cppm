export module toria.uuid;
#ifdef __INTELLISENSE__
#include "generators.cppm"
#include "impl.cppm"
#else
import toria.crypto;
export import :impl;
export import :generators;
export import :std;
#endif  // __INTELLISE__

namespace toria
{
	namespace uuid
	{
		// Nil uuid
		export constexpr uuid nil{"00000000-0000-0000-0000-000000000000"};
		// Max uuid
		export constexpr uuid max{"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"};
		// Name string is a fully-qualified domain name
		export constexpr uuid namespace_dns{"6ba7b810-9dad-11d1-80b4-00c04fd430c8"};
		// Name string is a URL
		export constexpr uuid namespace_url{"6ba7b811-9dad-11d1-80b4-00c04fd430c8"};
		// Name string is an ISO OID
		export constexpr uuid namespace_oid{"6ba7b812-9dad-11d1-80b4-00c04fd430c8"};
		// Name string is an X.500 DN (in DER or a text output format)
		export constexpr uuid namespace_x500{"6ba7b814-9dad-11d1-80b4-00c04fd430c8"};

		// export using v1 = generators::v1_generator;
		//export using v3 = generators::name_generator<3, crypto::md5>;
		export using v4 = generators::v4_generator<std::mt19937_64>;
		export using v5 = generators::name_generator<uuid::version_type::v5, crypto::sha1>;
		export using v7 = generators::v7_generator<std::mt19937_64>;
	}  // namespace uuid
}  // namespace toria
