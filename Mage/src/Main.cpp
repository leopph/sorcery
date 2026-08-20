#include <exception>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>

#include "command_line_processor.hpp"
#include "editor_app.hpp"
#include "Platform.hpp"


extern "C" {
__declspec(dllexport) extern UINT const D3D12SDKVersion{D3D12_SDK_VERSION};
__declspec(dllexport) extern char const* const D3D12SDKPath{".\\D3D12\\"};
}


#ifdef NDEBUG
auto WINAPI wWinMain([[maybe_unused]] _In_ HINSTANCE, [[maybe_unused]] _In_opt_ HINSTANCE,
                     _In_ wchar_t* const lpCmdLine, [[maybe_unused]] _In_ int) -> int {
#else
auto main(int argc, char* argv[]) -> int {
#endif
  try {
#ifdef NDEBUG
    sorcery::mage::CommandLineProcessor const cmd_proc{lpCmdLine};
#else
    sorcery::mage::CommandLineProcessor const cmd_proc{argc, argv};
#endif

#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    auto const msvc_sink{std::make_shared<spdlog::sinks::msvc_sink_mt>()};
    auto const logger{std::make_shared<spdlog::logger>("vs_logger", msvc_sink)};
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
#endif

    auto const args{cmd_proc.GetArgs()};

    if (auto const it{std::ranges::find_if(args, [](auto const arg) { return arg.starts_with("--log-level="); })};
      it != args.end()) {
      if (auto const log_level_arg{it->substr(12)};
        log_level_arg == "trace") {
        spdlog::set_level(spdlog::level::trace);
      } else if (log_level_arg == "debug") {
        spdlog::set_level(spdlog::level::debug);
      } else if (log_level_arg == "info") {
        spdlog::set_level(spdlog::level::info);
      } else if (log_level_arg == "warn") {
        spdlog::set_level(spdlog::level::warn);
      } else if (log_level_arg == "error") {
        spdlog::set_level(spdlog::level::err);
      } else if (log_level_arg == "critical") {
        spdlog::set_level(spdlog::level::critical);
      } else if (log_level_arg == "off") {
        spdlog::set_level(spdlog::level::off);
      }
    }

    sorcery::mage::EditorApp app{args};
    app.Run();
  } catch (std::exception const& ex) {
    sorcery::DisplayError(ex.what());
  }
  return 0;
}
