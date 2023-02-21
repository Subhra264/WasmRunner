#ifndef WASMRUNNER_HEADER

#define WASMRUNNER_HEADER

#include <wasmedge/wasmedge.h>
#include "iostream"
#include "vector"
#include "functional"

class WasmRunner {
   public:
      using SuccessHandler = std::function<int(std::vector<std::string> returns)>;
      using ErrorHandler = std::function<int(std::string msg)>;

      WasmRunner(SuccessHandler, ErrorHandler);
      ~WasmRunner();

      static const char *GetVersion();
      bool LoadWasmFile(std::string file_name);
      bool ValidateVM();
      bool InstantiateVM();
      int RunWasm(std::vector<std::string> &params, bool reactor_enabled, std::string entry_func);

   private:
      WasmEdge_VMContext *mVMCxt;
      WasmEdge_ConfigureContext *mConfCxt;

      SuccessHandler mSuccessFunctor;
      ErrorHandler mFailureFunctor;

      int ExecuteEntryFunc(std::vector<std::string> &params, const WasmEdge_String &entry_func);
      int RunWasmReactor(std::vector<std::string> &params, std::string entry_func);
      int RunWasmCommand(std::vector<std::string> &params);
};

#endif // WASMRUNNER_HEADER