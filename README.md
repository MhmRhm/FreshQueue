# The CMakeProjectTemplate
A well-structured modern cmake template for new projects providing testing with coverage, formatting along cppcheck, benchmarking, documentation generation, and more.

The contents of `./.devcontainer` are used to create a cross-platform containerized development environment using the provided `Dockerfile`. If you are using vscode, look into *Remote Development* extension.

- Read the `./INSTALL` and provide dependencies.
- Change `MY_PROJECT_NAME` and `MY_PACKAGE_NAME` variables at `./CMakeLists.txt`.
- Add your name and email to the `./cmake/Install.cmake` file.
- You should be good to go!
- In-source builds are not allowed and disabled(see `./cmake/NoInSourceBuilds.cmake`). Use as follows:

```bash
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -B ../build-llvm-debug -S .
cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -B ../build-gcc-debug -S .
cmake --build ../build-gcc-debug
cmake --build ../build-llvm-debug
```
