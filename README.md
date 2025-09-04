# What is junco?
Junco is a small, configurable engine for making video games in modern C++!

# Why junco?
- Junco is *minimal*
    - Junco does not include any unnecessary functionality into the engine by default. If you don't need something for your game, you don't have to include it. This keeps executables small, and updates fast!
- Junco is *modular*
    - In junco, modules define the engine's behavior. Modules can be added or removed in the engine's configuration. It's up to you to decide what your engine can do!
- Junco is *multithreaded*
    - Junco uses a centralized thread pool to manage concurrent operations. Heavy operations like module updates and file processing are all handled asynchronously to maximize efficiency!

# Building from Source
Requirements:
- CMake v3.40 or greater
- A compiler that supports C++20


Run the following from the project's root directory to generate a release build:
```
cd build
cmake ..
cmake build .
```
For more information about configuration flags, see the [documentation page](docs/building.md)

# Development Progress
See the project's [trello page]() for a more detailed overview.
- [x] Project setup
- [ ] Common types and utilities
- [ ] Engine core
- [ ] Multithreading
- [ ] Lua scripting
- [ ] Entities and components
- [ ] Levels
- [ ] Rendering
- [ ] Input
- [ ] Physics
- [ ] Audio