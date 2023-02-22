#include "WasmRunner.hh"
#include "finally.hh"

WasmRunner::WasmRunner(SuccessHandler successFunctor, ErrorHandler errorFunctor)
: mConfCxt(WasmEdge_ConfigureCreate()),
  mSuccessFunctor(successFunctor),
  mFailureFunctor(errorFunctor)
{
   WasmEdge_ConfigureAddHostRegistration(mConfCxt,
                                      WasmEdge_HostRegistration_Wasi);
   mVMCxt = WasmEdge_VMCreate(mConfCxt, NULL);
}

WasmRunner::~WasmRunner()
{
   WasmEdge_VMDelete(mVMCxt);
   WasmEdge_ConfigureDelete(mConfCxt);
}

const char *WasmRunner::GetVersion()
{
   return WasmEdge_VersionGet();
}

bool WasmRunner::LoadWasmFile(std::string file_name)
{
   WasmEdge_Result Result = WasmEdge_VMLoadWasmFromFile(mVMCxt, file_name.c_str());
   // file = file_name;

   if (!WasmEdge_ResultOK(Result))
      return !mFailureFunctor(WasmEdge_ResultGetMessage(Result));
   return true;
}

bool WasmRunner::ValidateVM()
{
   WasmEdge_Result Result = WasmEdge_VMValidate(mVMCxt);

   if (!WasmEdge_ResultOK(Result))
      return !mFailureFunctor(WasmEdge_ResultGetMessage(Result));
   return true;
}

bool WasmRunner::InstantiateVM()
{
   WasmEdge_Result Result = WasmEdge_VMInstantiate(mVMCxt);

   if (!WasmEdge_ResultOK(Result))
      return !mFailureFunctor(WasmEdge_ResultGetMessage(Result));
   return true;
}

int WasmRunner::RunWasm(std::vector<std::string> &params, bool reactor_enabled, std::string entry_func = "")
{
   if (reactor_enabled) return RunWasmReactor(params, entry_func);

   return RunWasmCommand(params);
}

int WasmRunner::ExecuteEntryFunc(std::vector<std::string> &params, const WasmEdge_String &EntryFuncName)
{
   const WasmEdge_FunctionTypeContext *FuncType =
      WasmEdge_VMGetFunctionType(mVMCxt, EntryFuncName);

   if (FuncType == NULL) return mFailureFunctor("Entry function not found!");

   const uint32_t BUFFER_LENGTH = 62; // As we are handling only a total of 64 arguments
   std::vector<WasmEdge_ValType> ParamTypeList(BUFFER_LENGTH);
   std::vector<WasmEdge_ValType> ReturnTypeList(BUFFER_LENGTH);

   const uint32_t ParamsLength =
      WasmEdge_FunctionTypeGetParameters(FuncType, ParamTypeList.data(), BUFFER_LENGTH);
   const uint32_t ReturnsLength =
      WasmEdge_FunctionTypeGetReturns(FuncType, ReturnTypeList.data(), BUFFER_LENGTH);

   if (ParamsLength > params.size()) {
      return mFailureFunctor("Too few arguments given!");
   }

   std::vector<WasmEdge_Value> Params(ParamsLength);
   std::vector<WasmEdge_Value> Returns(ReturnsLength);

   for(uint32_t i = 0; i < ParamsLength; i++) {

      try {
         switch (ParamTypeList[i])
         {
            case WasmEdge_ValType::WasmEdge_ValType_I32:
               // All int32_t values can be safely stored
               // as a long value (4 bytes for 32 bit or 8 bytes for 64 bit systems)
               Params[i] = WasmEdge_ValueGenI32(static_cast<int32_t>(std::stol(params[i])));
               break;
            case WasmEdge_ValType::WasmEdge_ValType_I64:
               Params[i] = WasmEdge_ValueGenI64(std::stol(params[i]));
               break;
            case WasmEdge_ValType::WasmEdge_ValType_F32:
               Params[i] = WasmEdge_ValueGenF32(std::stof(params[i]));
               break;
            case WasmEdge_ValType::WasmEdge_ValType_F64:
               Params[i] = WasmEdge_ValueGenF64(std::stod(params[i]));
               break;
            default:
               return mFailureFunctor("No other function parameter types supported for now!");
         }
      } catch(...) {
         return mFailureFunctor("Error parsing the passed arguments!");
      }
   }

   WasmEdge_Result Result = WasmEdge_VMExecute(mVMCxt,
                     EntryFuncName,
                     Params.data(),
                     ParamsLength,
                     Returns.data(),
                     ReturnsLength);

   // WasmEdge_Result Result = WasmEdge_VMRunWasmFromFile(mVMCxt,
   //                            file.c_str(),
   //                            EntryFuncName,
   //                            Params.data(),
   //                            ParamsLength,
   //                            Returns.data(),
   //                            ReturnsLength);

   // WasmEdge_Async *Async = WasmEdge_VMAsyncExecute(mVMCxt,
   //                   EntryFuncName,
   //                   Params.data(),
   //                   ParamsLength);

   // WasmEdge_Result Result = WasmEdge_AsyncGet(Async,
   //                   Returns.data(),
   //                   ReturnsLength);

   // finally([&Async]() {
   //    WasmEdge_AsyncDelete(Async);
   // });

   if (!WasmEdge_ResultOK(Result)) {
      return mFailureFunctor(WasmEdge_ResultGetMessage(Result));
   }

   std::vector<std::string> returns(ReturnsLength);

   for(uint32_t i = 0; i < ReturnsLength; i++) {
      switch (ReturnTypeList[i])
      {
         case WasmEdge_ValType::WasmEdge_ValType_I32:
            returns.push_back(std::to_string(WasmEdge_ValueGetI32(Returns[i])));
            break;
         case WasmEdge_ValType::WasmEdge_ValType_I64:
            returns.push_back(std::to_string(WasmEdge_ValueGetI64(Returns[i])));
            break;
         case WasmEdge_ValType::WasmEdge_ValType_F32:
            returns.push_back(std::to_string(WasmEdge_ValueGetF32(Returns[i])));
            break;
         case WasmEdge_ValType::WasmEdge_ValType_F64:
            returns.push_back(std::to_string(WasmEdge_ValueGetF64(Returns[i])));
            break;
         default:
            return mFailureFunctor("No other function return types supported for now!");
      }
   }

   return mSuccessFunctor(std::move(returns));
}

int WasmRunner::RunWasmReactor(std::vector<std::string> &params, std::string entry_func)
{
   const WasmEdge_String EntryFuncName = WasmEdge_StringCreateByCString(entry_func.c_str());
   finally delete_func_name([&EntryFuncName]() {
      WasmEdge_StringDelete(EntryFuncName);
   });

   return ExecuteEntryFunc(params, EntryFuncName);
}

int WasmRunner::RunWasmCommand(std::vector<std::string> &params)
{
   const uint32_t BUF_LEN = 1; // Assuming there is only one funciton to execute
   WasmEdge_String FuncNames[BUF_LEN];
   WasmEdge_FunctionTypeContext *FuncTypes[BUF_LEN];

   uint32_t RealFuncNum =
      WasmEdge_VMGetFunctionList(mVMCxt, FuncNames, (const WasmEdge_FunctionTypeContext **)FuncTypes, BUF_LEN);

   return ExecuteEntryFunc(params, FuncNames[0]);
}