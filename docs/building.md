Building the tests and examples
====
*streams* is a header-only library. That means no build-step is necessary if you just want to use the streams library - just clone / download the repository and add `[path to your copy of streams]/include` to the include paths of your project.

If you want to build and run the unit-tests and / or examples that come with the library however, a bit of extra work is needed:

1. **Install python 3**
The *streams* project uses *conan* as it's C++ package-manager. *Conan* itself is written in python, so you will need a python 3 version installed on your system. Please refer to [python.org](https://www.python.org) for details.
Once python 3 is installed, I recommend to create a virtual environment. That is not strictly necessary, but good practice as you can trash your python environment at any time and start fresh again. In the terminal / console navigate to where you want to create your virtual environment and run the following commands to create and activate it:
```bash
> python3 -m venv conan_virtual_env
# run the following line on linux/macos
> source conan_virtual_env/bin/activate
# run the following line on windows
> conan_virtual_env/Scripts/activate
```

2. **Install conan**
Now it's time to install conan itself. That's easy:
```bash
(conan_virtual_env) > pip install conan
```

3. **Install cmake**
For cross-platform builds, *streams* uses cmake as it's build-tool. Download and install the current cmake executable for your platform from [here](https://cmake.org/download/).

4. **Create your project**
Now it's time to create your project files. You should create a dedicated folder for your build to keep your directory structure clean. Let's assume you want to create the project files and build artifacts in a subfolder `build` of your *streams* installation. To do so navigate to the *streams* root folder and run the following commands:
```bash
(conan_virtual_env) > mkdir build
(conan_virtual_env) > cd build
# let conan download all dependencies as defined in conanfile.txt
(conan_virtual_env) > conan install ..
# use cmake to create the project files. 
# you can obtain a list of all available generators by typing "cmake -G"
(conan_virtual_env) > cmake .. -G [the generator of your choice]
```

5. **Build the tests and examples**
Depending on your choice for the cmake generator you will now find the project files for building the tests and examples in the [build] folder. After you built the binaries, those will reside in  [build/bin].

