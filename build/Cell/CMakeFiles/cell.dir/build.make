# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/chengling/Research/Project/Cell/3dVertex/tvm

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/chengling/Research/Project/Cell/3dVertex/tvm/build

# Include any dependencies generated for this target.
include Cell/CMakeFiles/cell.dir/depend.make

# Include the progress variables for this target.
include Cell/CMakeFiles/cell.dir/progress.make

# Include the compile flags for this target's objects.
include Cell/CMakeFiles/cell.dir/flags.make

Cell/CMakeFiles/cell.dir/Cell.cpp.o: Cell/CMakeFiles/cell.dir/flags.make
Cell/CMakeFiles/cell.dir/Cell.cpp.o: ../Cell/Cell.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/chengling/Research/Project/Cell/3dVertex/tvm/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object Cell/CMakeFiles/cell.dir/Cell.cpp.o"
	cd /home/chengling/Research/Project/Cell/3dVertex/tvm/build/Cell && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cell.dir/Cell.cpp.o -c /home/chengling/Research/Project/Cell/3dVertex/tvm/Cell/Cell.cpp

Cell/CMakeFiles/cell.dir/Cell.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cell.dir/Cell.cpp.i"
	cd /home/chengling/Research/Project/Cell/3dVertex/tvm/build/Cell && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/chengling/Research/Project/Cell/3dVertex/tvm/Cell/Cell.cpp > CMakeFiles/cell.dir/Cell.cpp.i

Cell/CMakeFiles/cell.dir/Cell.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cell.dir/Cell.cpp.s"
	cd /home/chengling/Research/Project/Cell/3dVertex/tvm/build/Cell && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/chengling/Research/Project/Cell/3dVertex/tvm/Cell/Cell.cpp -o CMakeFiles/cell.dir/Cell.cpp.s

# Object files for target cell
cell_OBJECTS = \
"CMakeFiles/cell.dir/Cell.cpp.o"

# External object files for target cell
cell_EXTERNAL_OBJECTS =

Cell/libcell.a: Cell/CMakeFiles/cell.dir/Cell.cpp.o
Cell/libcell.a: Cell/CMakeFiles/cell.dir/build.make
Cell/libcell.a: Cell/CMakeFiles/cell.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/chengling/Research/Project/Cell/3dVertex/tvm/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libcell.a"
	cd /home/chengling/Research/Project/Cell/3dVertex/tvm/build/Cell && $(CMAKE_COMMAND) -P CMakeFiles/cell.dir/cmake_clean_target.cmake
	cd /home/chengling/Research/Project/Cell/3dVertex/tvm/build/Cell && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cell.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
Cell/CMakeFiles/cell.dir/build: Cell/libcell.a

.PHONY : Cell/CMakeFiles/cell.dir/build

Cell/CMakeFiles/cell.dir/clean:
	cd /home/chengling/Research/Project/Cell/3dVertex/tvm/build/Cell && $(CMAKE_COMMAND) -P CMakeFiles/cell.dir/cmake_clean.cmake
.PHONY : Cell/CMakeFiles/cell.dir/clean

Cell/CMakeFiles/cell.dir/depend:
	cd /home/chengling/Research/Project/Cell/3dVertex/tvm/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/chengling/Research/Project/Cell/3dVertex/tvm /home/chengling/Research/Project/Cell/3dVertex/tvm/Cell /home/chengling/Research/Project/Cell/3dVertex/tvm/build /home/chengling/Research/Project/Cell/3dVertex/tvm/build/Cell /home/chengling/Research/Project/Cell/3dVertex/tvm/build/Cell/CMakeFiles/cell.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : Cell/CMakeFiles/cell.dir/depend

