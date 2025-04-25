# Okay so first selecting linux's g++ compiler to be used in this case...
CXX = g++

# Compiler flags I am using
# -std=c++17: using the C++ 17 standard which is the default in our lab computers ( so hopefully maintaains 100% compatibility)
# -Wall: Enable all compiler warnings (this is for me, ignore)
# -Wextra: Enable extra warnings to catch potential issues. (this is also for me, pls ignore)
# Enhancing code performacine (apparently it works??????!!!!) using -02

CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# The name of the executable file to create
TARGET = main

# The source file to compile
SRC = main.cpp

# Default rule (target) for building the program; This compiles the source file into the executable; 
#in this case "main" file. and then commands like ./main input(number).txt can be used
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Adding a clean rule (as indicated in week5 and assignment 1); not adding deepclean as I don't think it's required.
# This will remove the generated "main" file
clean:
	rm -f $(TARGET)
