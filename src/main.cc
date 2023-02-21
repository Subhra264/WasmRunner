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
   std::cout << std::endl;
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

      if (args[0] == "version") {
         std::cout << WasmRunner::GetVersion() << std::endl;
         return 0;
      } else if (args[0] == "-h" || args[0] == "--help") {
         PrintHelp();
         return 0;
      } else if (args[0] == "--reactor") {
         reactor_enabled = true;
         file_name_index = 1;
      }

      if (args.size() <= file_name_index) return PrintError("File name must be given!");

      if (args[file_name_index] == "run") file_name_index += 1;

      if (args.size() > file_name_index) {
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

      return wasmRunner->RunWasm(params, reactor_enabled, entry_func);

   }

   return PrintError("Too many arguments!");
}
