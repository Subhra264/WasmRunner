#include "iostream"
#include "WasmRunner.hh"
#include "memory"
#include "vector"
#include "algorithm"

int PrintError(std::string msg) {
   std::cerr << msg << std::endl;
   return 1;
}

void PrintHelp() {

}

int PrintResult(std::vector<std::string> returns) {
   for(auto result: returns) {
      std::cout << result << " ";
   }
   return 0;
}

int main(int Argc, const char* Argv[]) {

   if (Argc == 1) {
      return PrintError("Atleast one argument is required!\nRun with -h or --help for help");
   } else if (Argc >= 2 && Argc <= 64) {
      // Ignore the first argument which is the tool name itself
      std::vector<std::string> args(Argv + 1, Argv + Argc);
      std::string entry_func = "";
      bool reactor_enabled = false;
      int file_name_index = 0;
      int file_params_index = file_name_index + 1;
      // int entry_func_index = -1;

      if (args[0] == "version") {
         std::cout << WasmRunner::GetVersion() << std::endl;
         return 0;
      } else if (args[0] == "-h" || args[0] == "--help") {
         PrintHelp();
         return 0;
      } else if (args[0] == "--reactor") {
         reactor_enabled = true;
         file_name_index = 1;
         // entry_func_index = file_name_index + 1;
      }

      if (args.size() <= file_name_index) return PrintError("File name must be given!");

      if (args[file_name_index] == "run") file_name_index += 1;

      if (args.size() > file_name_index) {
         // file_name = args[file_name_index];
         file_params_index = file_name_index + 1;

         if (reactor_enabled) {
            if (file_params_index < args.size()) {
               entry_func = args[file_params_index];
               file_params_index += 1;
            } else {
               return PrintError("Entry function name must be given while running with --reactor flag");
            }
         }
      } else {
         return PrintError("File name must be given!");
      }

      std::string file_name = args[file_name_index];
      std::vector<std::string> params;

      if (file_params_index < args.size()) {
         std::for_each(args.begin() + file_params_index, args.end(), [&params](const std::string arg) {
            params.push_back(arg);
         });   
      }

      std::unique_ptr<WasmRunner> wasmRunner(new WasmRunner(PrintResult, PrintError));
      wasmRunner->LoadWasmFile(file_name);
      wasmRunner->ValidateVM();
      wasmRunner->InstantiateVM();

      wasmRunner->RunWasm(params, reactor_enabled, entry_func);

   } else {
      return PrintError("Too many arguments!");
   }

   // std::cout<<Argc<<"  "<<Argv[0]<<std::endl;
   // std::cout<<"WasmEdge version: "<<WasmRunner::GetVersion()<<std::endl;

   // WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
   // WasmEdge_ConfigureAddHostRegistration(ConfCxt, WasmEdge_HostRegistration_Wasi);
   // /* The configure and store context to the VM creation can be NULL. */
   // WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(ConfCxt, NULL);

   // /* The parameters and returns arrays. */
   // WasmEdge_Value Params[1] = { WasmEdge_ValueGenI32(32) };
   // WasmEdge_Value Returns[1];
   // /* Function name. */
   // WasmEdge_String FuncName = WasmEdge_StringCreateByCString("fib");
   // /* Run the WASM function from file. */
   // WasmEdge_Result Res = WasmEdge_VMRunWasmFromFile(VMCxt, Argv[1], FuncName, Params, 1, Returns, 1);

   // if (WasmEdge_ResultOK(Res)) {
   //    std::cout<<"Get result: "<<WasmEdge_ValueGetI32(Returns[0])<<std::endl;
   // } else {
   //    std::cout<<"Error message: "<<WasmEdge_ResultGetMessage(Res)<<std::endl;
   // }

   // /* Resources deallocations. */
   // WasmEdge_VMDelete(VMCxt);
   // WasmEdge_ConfigureDelete(ConfCxt);
   // WasmEdge_StringDelete(FuncName);
   // std::cout<<"Main is working!"<<std::endl;
   
   return 0;
}
