module;
#include <windows.h>
#include <Processthreadsapi.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <dxgi1_6.h>
#include <winrt/Windows.Foundation.Collections.h>
module toria.system;
import toria.util;

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "dxgi.lib")

namespace toria::system
{
	namespace system_info
	{

		struct IWbemServicesDeleter
		{
			void operator()(IWbemServices* pSvc) const {
				if (pSvc)
					pSvc->Release();
			}
		};

		struct IWbemLocatorDeleter
		{
			void operator()(IWbemLocator* pLoc) const {
				if (pLoc) {
					pLoc->Release();
				}
			}
		};

		class wbem
		{
		public:
			wbem() noexcept {
				HRESULT hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
				if (!FAILED(hres)) {
					m_initialised = true;
				}
				init();
			}

			IWbemLocator* locator() const noexcept {
				return m_locator.get();
			}

			IWbemServices* services() const noexcept {
				return m_services.get();
			}

			~wbem() noexcept {
				if (m_initialised) {
					m_services.reset();
					m_locator.reset();
					CoUninitialize();
				}
			}

		private:
			void init() {
				HRESULT hres = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
				                                    RPC_C_AUTHN_LEVEL_DEFAULT,
				                                    RPC_C_IMP_LEVEL_IMPERSONATE,
				                                    nullptr, EOAC_NONE, nullptr);
				if (FAILED(hres))
					return;
				IWbemLocator* pLoc = nullptr;
				hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
				                        IID_IWbemLocator,
				                        reinterpret_cast<LPVOID*>(&pLoc));
				if (FAILED(hres))
					return;
				m_locator.reset(pLoc);
				IWbemServices* pSvc = nullptr;
				hres = m_locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr,
				                                0,
				                                nullptr, nullptr, &pSvc);
				m_services.reset(pSvc);
				if (FAILED(hres))
					return;
				static_cast<void>(CoSetProxyBlanket(m_services.get(),RPC_C_AUTHN_WINNT,
				                                    RPC_C_AUTHZ_NONE,
				                                    nullptr,
				                                    RPC_C_AUTHN_LEVEL_CALL,
				                                    RPC_C_IMP_LEVEL_IMPERSONATE,
				                                    nullptr, EOAC_NONE));
			}

			std::unique_ptr<IWbemLocator, IWbemLocatorDeleter> m_locator = nullptr;
			std::unique_ptr<IWbemServices, IWbemServicesDeleter> m_services = nullptr;
			bool m_initialised = false;
		};

		wbem& get_wbem_instance() noexcept {
			static wbem instance = []() -> wbem {
				return {};
			}();
			return instance;
		}

		std::string get_string_property(IWbemClassObject* classObject, VARIANT& prop,
		                                const std::wstring_view propName) {
			static_cast<void>(classObject->Get(propName.data(), 0, &prop, nullptr, nullptr));
			const auto val = std::string(util::trim(util::from_wide_string(prop.bstrVal)));
			static_cast<void>(VariantClear(&prop));
			return val;
		}

		bool get_bool_property(IWbemClassObject* classObject, VARIANT& prop,
		                       const std::wstring_view propName) {
			static_cast<void>(classObject->Get(propName.data(), 0, &prop, nullptr, nullptr));
			const bool val = prop.boolVal;
			static_cast<void>(VariantClear(&prop));
			return val;
		}

		std::uint32_t get_uint32_property(IWbemClassObject* classObject,VARIANT& prop,const std::wstring_view propName) {
			static_cast<void>(classObject->Get(propName.data(), 0, &prop, nullptr, nullptr));
			const std::uint32_t val = prop.uintVal;
			static_cast<void>(VariantClear(&prop));
			return val;
		}

		std::uint16_t get_uint16_property(IWbemClassObject* classObject,VARIANT& prop,const std::wstring_view propName) {
			static_cast<void>(classObject->Get(propName.data(), 0, &prop, nullptr, nullptr));
			const std::uint16_t val = prop.uiVal;
			static_cast<void>(VariantClear(&prop));
			return val;
		}

		os_info get_os_info() noexcept {
			static os_info instance = []() -> os_info {
				os_info out{};
				auto services = get_wbem_instance().services();
				if (services != nullptr) {
					IEnumWbemClassObject* enumerator = nullptr;
					HRESULT hres = services->ExecQuery(
						bstr_t("WQL"),bstr_t("SELECT * FROM Win32_OperatingSystem"),
						WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
						&enumerator);
					if (FAILED(hres))
						return {};
					IWbemClassObject* classObject = nullptr;
					ULONG uReturn = 0;
					while (enumerator) {
						static_cast<void>(enumerator->
							Next(WBEM_INFINITE, 1, &classObject, &uReturn));
						if (uReturn == 0)
							break;
						VARIANT prop;
						VariantInit(&prop);
						out.architecture =
							get_string_property(classObject, prop, L"OSArchitecture");
						out.version = get_string_property(classObject, prop, L"Version");
						out.build_number = get_string_property(classObject, prop, L"BuildNumber");
						out.portable = get_bool_property(classObject, prop,
						                                 L"PortableOperatingSystem");
						classObject->Release();
					}
					enumerator->Release();
				}
				return out;
			}();
			return instance;
		}

		std::string get_processor_architecture_string(const std::uint16_t architecture) noexcept {
			switch (architecture) {
				case 0:
					return "x86";
				case 1:
					return "MIPS";
				case 2:
					return "Alpha";
				case 3:
					return "PowerPC";
				case 5:
					return "ARM";
				case 6:
					return "ia64";
				case 9:
					return "x64";
				case 12:
					return "ARM64";
				default:
					return "Unknown Architecture";
			}
		}

		processor_info get_processor_info() noexcept {
			static processor_info info = []() -> processor_info {
				processor_info out{};
				auto services = get_wbem_instance().services();
				if (services != nullptr) {
					IEnumWbemClassObject* enumerator = nullptr;
					HRESULT hres = services->ExecQuery(
						bstr_t("WQL"),bstr_t("SELECT * FROM Win32_Processor"),
						WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
						&enumerator);
					if (FAILED(hres))
						return {};
					IWbemClassObject* classObject = nullptr;
					ULONG uReturn = 0;
					while (enumerator) {
						static_cast<void>(enumerator->
							Next(WBEM_INFINITE, 1, &classObject, &uReturn));
						if (uReturn == 0)
							break;
						VARIANT prop;
						VariantInit(&prop);
						out.name = get_string_property(classObject,prop,L"Name");
						out.socket = get_string_property(classObject, prop, L"SocketDesignation");
						out.architecture = get_processor_architecture_string(get_uint16_property(classObject,prop,L"Architecture"));
						out.manufacturer = get_string_property(classObject,prop,L"Manufacturer");
						out.cores = get_uint32_property(classObject,prop,L"NumberOfCores");
						out.enabled_cores = get_uint32_property(classObject,prop,L"NumberOfEnabledCore");
						out.logical_processors = get_uint32_property(classObject,prop,L"NumberOfLogicalProcessors");
						out.address_width = get_uint16_property(classObject, prop,L"AddressWidth");
						classObject->Release();
					}
					enumerator->Release();
				}
				return out;
			}();
			return info;
		}

		gpu_info get_gpu_info() noexcept {
				static gpu_info info = []() -> gpu_info {
					winrt::com_ptr<IDXGIFactory> factory;
					CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
					IDXGIAdapter* adapter;
					factory->EnumAdapters(0, &adapter);
					gpu_info out{};
					auto services = get_wbem_instance().services();
					if (services != nullptr) {
						IEnumWbemClassObject* enumerator = nullptr;
						HRESULT hres = services->ExecQuery(
							bstr_t("WQL"),bstr_t("SELECT * FROM Win32_VideoController"),
							WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
							&enumerator);
						if (FAILED(hres))
							return {};
						IWbemClassObject* classObject = nullptr;
						ULONG uReturn = 0;
						while (enumerator) {
							static_cast<void>(enumerator->
								Next(WBEM_INFINITE, 1, &classObject, &uReturn));
							if (uReturn == 0)
								break;
							VARIANT prop;
							VariantInit(&prop);
							out.name = get_string_property(classObject,prop,L"Name");
							out.driver_version = get_string_property(classObject,prop,L"DriverVersion");
							out.vram = get_uint32_property(classObject,prop,L"AdapterRAM");
							classObject->Release();
						}
						enumerator->Release();
					}
					return out;
				}();
				return info;
		}
	}

	std::uint64_t process::get_current_id() noexcept {
		static std::uint64_t current_process_id = []() -> std::uint64_t {
			const DWORD pid = GetCurrentProcessId();
			return pid;
		}();
		return current_process_id;
	}

	static std::uint64_t fromFILETIME(const FILETIME* ft) noexcept {
		return util::fromHighLow(ft->dwHighDateTime, ft->dwLowDateTime);
	}

	std::chrono::time_point<std::chrono::file_clock> process::get_creation_time() noexcept {
		using time_point = std::chrono::time_point<std::chrono::file_clock>;
		static auto creation_time = []() -> time_point {
			if (const auto hProcess = GetCurrentProcess(); hProcess != nullptr) {
				FILETIME creationTime{}, dump{};
				if (GetProcessTimes(hProcess, &creationTime, &dump, &dump, &dump)) {
					return time_point{time_point::duration(fromFILETIME(&creationTime))};
				}
			}
			return {};
		}();
		return creation_time;
	}

	process::usage process::get_usage_time() noexcept {
		using time_point = std::chrono::time_point<std::chrono::file_clock>;
		if (const auto hProcess = GetCurrentProcess(); hProcess != nullptr) {
			FILETIME kernelTime, userTime, dump;
			if (GetProcessTimes(hProcess, &dump, &dump, &kernelTime, &userTime)) {
				return {
					time_point{time_point::duration(fromFILETIME(&kernelTime))},
					time_point{time_point::duration(fromFILETIME(&userTime))}
				};
			}
		}
		return {};
	}

	unique_pid_token
	unique_pid_token::get_unique_pid_token_for_process(const std::uint64_t pid) noexcept {
		using time_point = std::chrono::time_point<std::chrono::file_clock>;
		if (const auto hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);
			hProcess != nullptr) {
			FILETIME creationTime, dump;
			if (GetProcessTimes(hProcess, &creationTime, &dump, &dump, &dump)) {
				return {pid, time_point{time_point::duration(fromFILETIME(&creationTime))}};
			}
		}
		return {-1ull, {}};
	}

	// NOLINTNEXTLINE(misc-misplaced-const)
	std::string native_get_thread_name(const HANDLE hThread) noexcept {
		PWSTR data = nullptr;
		if (const auto hr = GetThreadDescription(hThread, &data); SUCCEEDED(hr)) {
			std::string out = util::from_wide_string(data);
			LocalFree(data);
			return out;
		}
		return "";
	}

	std::string get_thread_name(std::thread& thread) noexcept {
		if (!thread.native_handle())
			return "";
		if (auto res = native_get_thread_name(thread.native_handle()); !res.empty())
			return res;
		return std::format("{}", thread.get_id());
	}

	std::string thread::get_thread_name(const std::thread::id threadId) noexcept {
		const auto underlying = threadId._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, underlying);
		if (hThread == nullptr)
			return "";
		if (auto res = native_get_thread_name(hThread); !res.empty())
			return res;
		return std::format("{}", threadId);
	}

	// NOLINTNEXTLINE(misc-misplaced-const)
	bool native_set_thread_name(const HANDLE hThread, const std::string_view name) noexcept {
		return SUCCEEDED(SetThreadDescription(hThread, toria::util::to_wide_string(name).c_str()));
	}

	bool thread::set_thread_name(std::thread& thread, const std::string_view name) noexcept {
		if (!thread.native_handle())
			return false;
		return native_set_thread_name(thread.native_handle(), name);
	}

	bool
	thread::set_thread_name(const std::thread::id threadID, const std::string_view name) noexcept {
		const auto underlying = threadID._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, underlying);\
		if (hThread == nullptr)
			return false;
		return native_set_thread_name(hThread, name);
	}

	static thread::thread_priority _map_native_to_priority(const int priority) {
		switch (priority) {
			case THREAD_PRIORITY_IDLE:
				return thread::thread_priority::IDLE;
			case THREAD_PRIORITY_LOWEST:
				return thread::thread_priority::LOW;
			case THREAD_PRIORITY_BELOW_NORMAL:
				return thread::thread_priority::BELOW_NORMAL;
			case THREAD_PRIORITY_ABOVE_NORMAL:
				return thread::thread_priority::ABOVE_NORMAL;
			case THREAD_PRIORITY_HIGHEST:
				return thread::thread_priority::HIGH;
			case THREAD_PRIORITY_TIME_CRITICAL:
				return thread::thread_priority::REALTIME;
			case THREAD_PRIORITY_NORMAL:
			default:
				return thread::thread_priority::NORMAL;
		}
	}

	static int _map_priority_to_native(const thread::thread_priority priority) noexcept {
		switch (priority) {
			case thread::thread_priority::IDLE:
				return THREAD_PRIORITY_IDLE;
			case thread::thread_priority::LOW:
				return THREAD_PRIORITY_LOWEST;
			case thread::thread_priority::BELOW_NORMAL:
				return THREAD_PRIORITY_BELOW_NORMAL;
			case thread::thread_priority::ABOVE_NORMAL:
				return THREAD_PRIORITY_ABOVE_NORMAL;
			case thread::thread_priority::HIGH:
				return THREAD_PRIORITY_HIGHEST;
			case thread::thread_priority::REALTIME:
				return THREAD_PRIORITY_TIME_CRITICAL;
			case thread::thread_priority::NORMAL:
			default:
				return THREAD_PRIORITY_NORMAL;
		}
	}

	std::optional<thread::thread_priority>
	thread::get_thread_priority(std::thread& thread) noexcept {
		if (!thread.native_handle())
			return std::nullopt;
		const int priority = GetThreadPriority(thread.native_handle());
		return _map_native_to_priority(priority);
	}

	std::optional<thread::thread_priority> thread::get_thread_priority(
		const std::thread::id threadId) noexcept {
		const auto underlying = threadId._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, underlying);
		if (!hThread)
			return std::nullopt;
		const int priority = GetThreadPriority(hThread);
		return _map_native_to_priority(priority);
	}

	bool thread::set_thread_priority(std::thread& thread, const thread_priority priority) noexcept {
		if (!thread.native_handle())
			return false;
		return SUCCEEDED(
			SetThreadPriority(thread.native_handle(), _map_priority_to_native(priority)));
	}

	bool thread::set_thread_priority(const std::thread::id threadID,
	                                 const thread_priority priority) noexcept {
		const auto underlying = threadID._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, underlying);
		if (!hThread)
			return false;
		return SUCCEEDED(SetThreadPriority(hThread, _map_priority_to_native(priority)));
	}

	// NOLINTNEXTLINE(misc-misplaced-const)
	bool native_is_background_thread(const HANDLE hThread) noexcept {
		const auto setToBackground =
			SUCCEEDED(SetThreadPriority(hThread, THREAD_MODE_BACKGROUND_BEGIN));
		if (setToBackground) {
			SetThreadPriority(hThread, THREAD_MODE_BACKGROUND_END);
		}
		return !setToBackground;
	}

	bool is_background_thread(std::thread& thread) noexcept {
		if (!thread.native_handle())
			return false;
		return native_is_background_thread(thread.native_handle());
	}

	bool is_background_thread(const std::thread::id id) noexcept {
		const auto underlying = id._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, underlying);
		if (!hThread)
			return false;
		return native_is_background_thread(hThread);
	}

	// NOLINTNEXTLINE(misc-misplaced-const)
	bool native_switch_thread_mode(const HANDLE hThread, const bool setBackground) noexcept {
		if (setBackground)
			return SUCCEEDED(SetThreadPriority(hThread, THREAD_MODE_BACKGROUND_BEGIN));
		return SUCCEEDED(SetThreadPriority(hThread, THREAD_MODE_BACKGROUND_END));
	}

	bool switch_thread_mode(std::thread& thread, const bool setBackground) noexcept {
		if (!thread.native_handle())
			return false;
		return native_switch_thread_mode(thread.native_handle(), setBackground);
	}

	bool switch_thread_mode(const std::thread::id id, const bool setBackground) noexcept {
		const auto underlying = id._Get_underlying_id();
		const auto hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, underlying);
		if (!hThread)
			return false;
		return native_switch_thread_mode(hThread, setBackground);
	}
} // namespace toria::system
