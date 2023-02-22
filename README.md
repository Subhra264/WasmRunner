# WasmRunner

A command line tool to execute WebAssembly files.

```bash
./WasmRunner [version] [-h | --help] [--reactor] [run]
             [path_to_wasm_file] [entry_function] [args]
```

## Building
As this tool uses `wasmedge` C library as a dependency, it needs to be installed first. Refer to [this](https://wasmedge.org/book/en/quick_start/install.html) as a guide

You can install it with `git` and `curl` -
```bash
curl -sSf https://raw.githubusercontent.com/WasmEdge/WasmEdge/master/utils/install.sh | bash
```

After you install it, clone this repo -
```bash
git clone https://github.com/Subhra264/WasmRunner.git
```

Move to the `/WasmRunner` directory -
```bash
cd WasmRunner/
```

Now simply execute the `build.sh` file -
```bash
./build.sh
```

Once the `build` directory is created, move to the directory and use the tool -
```bash
cd build

./WasmRunner version
```

## How is this built?

This tool is completely based on the `wasmedge` C library SDK and is written in C++11. There are two files in the `/src` directory -
- `main.cc` - Responsible for executing all the CLI related logic i.e. taking inputs, doing initial parsing and validation of inputs and printing results or errors to the console.
- `WasmRunner.cc` - Implements the `WasmRunner` class that wraps the internally used `wasmedge` C library SDK. It exposes various methods necessary for executing a given wasm file.

## WasmRunner class

Public member functions -

### Constructor
Instantiates the `WasmRunner` class. Also initializes internally used `WasmEdge_ConfigureContext` and `WasmEdge_VMContext`

#### Params
- `SuccessHandler` - A success handler function to be called with returned values from the execution of given wasm file
- `ErrorHandler` - A error handler function to be called with the error message occured during the execution of given wasm file.

#### Returns
- `WasmRunner` - A new `WasmRunner` instance.

### static GetVersion()
A `static` member function that returns the `wasmedge` C library version as an array of `char`s.

#### Returns
- `const char *` -  The version of `wasmedge` C SDK.

### LoadWasmFile()
Loads the wasm file given using `WasmEdge_VMLoadWasmFromFile` C function.

#### Params
- `std::string file_name` - The wasm file path to load.

#### Returns
- `bool` - If true, the operation was successful, failed otherwise.

### ValidateVM()
Internally uses `WasmEdge_VMValidate` C function to validate the `VM` after loading wasm file.

#### Returns
- `bool` - If true, the operation was successful, failed otherwise.

### InstantiateVM()
Internally uses `WasmEdge_VMInstantiate` C function to instantiate the validated wasm module in the `VM` context.

#### Returns
- `bool` - If true, the operation was successful, failed otherwise.

### RunWasm()
Executes the given wasm file.

#### Params
- `std::vector<std::string>> params` - A vector with all the wasm file arguments.
- `bool reactor_enabled` - A boolean representing if reactor flag is on.
- `std::string entry_func` - If reactor flag is on, a valid entry function name must be given

#### Returns
- `int` - If `0`, the operation was successful and not successful otherwise.

## Commands
Following are various commands to use with this tool.

### Version
Prints the version of the underlying `wasmedge` library. All other commands and options are ignored when this command is run.

```bash
./WasmRunner version
```
![WasmRunner version](/assets/wasm_edge_version.png)
![WasmRunner version ignores all other options](/assets/wasm_runner_version_ignore_others.png)

### Help
Prints the manual page in the console. When specified all the other options are ignored.

```bash
./WasmRunner -h

./WasmRunner --help
```

![WasmRunner help](/assets/wasm_runner_help.png)

### Run
Optional `run` command followed by a `file path` to a WebAssembly file.

```bash
./WasmRunner run <file_path> [args]
```

![WasmRunner run](/assets/wasm_runner_run.png)

### Without run
You can also execute WebAssembly files without the run command. In this case the command will be -

```bash
./WasmRunner [--reactor] <file_path> [entry_function_name] [args]
```

![WasmRunner without run](/assets/wasm_runner_without_run.png)

### Reactor flag

When there are multiple funcitons exported by the given WebAssembly file, you can specify a entry function name to be executed. When using this flag, `entry_function_name` after `file_path` is required.

```bash
./WasmRunner --reactor <file_path> entry_function_name [args]
```

![WasmRunner with reactor flag](/assets/wasm_runner_with_reactor.png)

### Errors

When something errorneous occurs, the tool fails with `int` code `1` (indicating error) and prints the error message to the CLI console. Here are few of those errors that may occur while running this tool -

#### Too few arguments
User passes smaller number of parameters than what actually is required by the exported WebAssembly function.

![WasmRunner too few arguments error](/assets/wasm_runner_too_few_arguments_error.png)

#### Entry function not found
When using with `--reactor` flag, if wrong entry function name (not actually exported) is given, this error message is printed on the console.

![WasmRunner entry function not found](/assets/wasm_runner_entry_func_not_found.png)

![WasmRunner entry function wrongly given](/assets/wasm_runner_entry_func_not_found_wrong_name.png)

#### Entry function not given
When using with `--reactor` flag, if the user does not provide the entry function name at all after the `file_path`, this error message is printed.

![WasmRunner entry function not given](/assets/wasm_runner_entry_func_name_must_be_given.png)
