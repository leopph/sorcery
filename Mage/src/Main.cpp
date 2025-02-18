#include "command_line_processor.hpp"
#include "EditorApp.hpp"
#include "Platform.hpp"

#include <exception>


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
    sorcery::mage::EditorApp app{cmd_proc.GetArgs()};
    app.Run();
  } catch (std::exception const& ex) {
    sorcery::DisplayError(ex.what());
  }
  return 0;
}
