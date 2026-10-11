#pragma once

#include "sorcery/host/job_system.hpp"

#include <utility>


namespace sorcery::mage {
template<typename Callable>
auto EditorApp::ExecuteInBusyEditor(Callable&& callable) -> void {
  GetJobSystem().Submit(GetJobSystem().CreateJob([this, callable = std::forward<Callable>(callable)] {
    BusyExecutionContext const exec_context{OnEnterBusyExecution()};

    try {
      std::invoke(callable);
    } catch (std::exception const& ex) {
      HandleBackgroundThreadException(ex);
    } catch (...) {
      HandleUnknownBackgroundThreadException();
    }

    OnFinishBusyExecution(exec_context);
  }));
}
}
