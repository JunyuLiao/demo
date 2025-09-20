# Paths
INCLUDE_PATH = /opt/local/include
LIBRARY_PATH = /opt/local/lib

# Compiler - use g++ for Railway, clang++ for local
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CXX = g++
    # Try different GLPK locations on Linux
    ifeq ($(wildcard /usr/include/glpk.h),)
        ifeq ($(wildcard /usr/local/include/glpk.h),)
            INCLUDE_PATH = /usr/include
        else
            INCLUDE_PATH = /usr/local/include
            LIBRARY_PATH = /usr/local/lib
        endif
    else
        INCLUDE_PATH = /usr/include
        LIBRARY_PATH = /usr/lib
    endif
else
    CXX = clang++
    INCLUDE_PATH = /opt/local/include
    LIBRARY_PATH = /opt/local/lib
endif

# Compiler flags
CXXFLAGS = -w -I$(INCLUDE_PATH)
CXXFLAGS += --std=c++17 -Wall -Werror -pedantic -g
# Enable sanitizers to catch segfault root causes in production logs
CXXFLAGS += -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
LDFLAGS = -L$(LIBRARY_PATH) -lglpk -lm -fsanitize=address -fsanitize=undefined

# Target executables
TARGET = run
WEB_TARGET = run_web

# Build all
all:
	$(CXX) $(CXXFLAGS) main.cpp highdim.cpp attribute_subset.cpp util.cpp other/*.c other/*.cpp $(LDFLAGS) -Ofast -o $(TARGET)

# Build web version (real interactive)
web-real:
	$(CXX) $(CXXFLAGS) main_web_real.cpp highdim.cpp attribute_subset.cpp util_web.cpp other/*.c other/*.cpp $(LDFLAGS) -Ofast -o $(WEB_TARGET)

# Build with Valgrind
valgrind:
	$(CXX) $(CXXFLAGS) *.cpp other/*.c other/*.cpp $(LDFLAGS) -g -O0 -o $(TARGET)

# Build with GDB
gdb:
	$(CXX) -g $(CXXFLAGS) *.cpp other/*.c other/*.cpp $(LDFLAGS) -o $(TARGET)

# Clean up
clean:
	rm -f $(TARGET) $(WEB_TARGET)

.PHONY: clean