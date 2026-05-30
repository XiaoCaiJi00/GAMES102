#include <Utopia/App/Editor/Editor.h>

#include <Utopia/App/Editor/InspectorRegistry.h>

#include <UECS/World.h>

#include <Utopia/Core/StringsSink.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "Components/CanvasData.h"
#include "Systems/CanvasSystem.h"

#ifndef NDEBUG
#include <dxgidebug.h>
#endif

using namespace Ubpa::Utopia;

// 初始化日志系统
void InitLogger() {
  // 创建编辑器显示用的 StringsSink
  auto strings_sink = std::make_shared<Ubpa::Utopia::StringsSink>();

  // 创建控制台输出 sink
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // 创建 logger 并添加所有 sink
  auto logger = std::make_shared<spdlog::logger>(
    "UtopiaEditor",
    spdlog::sinks_init_list{ strings_sink, console_sink }
  );

  // 设置为默认 logger
  spdlog::set_default_logger(logger);

  // 设置日志级别（调试模式下显示所有级别）
#if defined(DEBUG) || defined(_DEBUG)
  spdlog::set_level(spdlog::level::trace);
#else
  spdlog::set_level(spdlog::level::info);
#endif

  // 设置输出格式（带颜色）
  spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) {
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    InitLogger();
	int rst;
    try {
        Editor app(hInstance);
        if(!app.Init())
            return 1;
        auto game = app.GetGameWorld();
        game->systemMngr.RegisterAndActivate<CanvasSystem>();
        game->entityMngr.cmptTraits.Register<CanvasData>();
        game->entityMngr.Create<CanvasData>();

		rst = app.Run();
    }
    catch(Ubpa::UDX12::Util::Exception& e) {
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        rst = 1;
	}

#ifndef NDEBUG
	Microsoft::WRL::ComPtr<IDXGIDebug> debug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug));
    if(debug)
	    debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
#endif

	return rst;
}
