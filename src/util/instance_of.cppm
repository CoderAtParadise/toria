export module toria.util:instance_of;
import std;

namespace toria::util
{
	export template<class, template<class...> class>
	constexpr bool instance_of_v = false;
	template<class... T, template<class...> class U>
	constexpr bool instance_of_v<U<T...>, U> = true;

	export template<class T, template<class...> class U>
	concept instance_of = instance_of_v<T, U>;
}
