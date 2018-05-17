QTUM_PATH = ../../
INCLUDES = -I$(QTUM_PATH)
CC = clang++
# CC = /usr/local/Cellar/gcc/6.1.0/bin/gcc-6

main: main.o
	$(CC) main.o $(QTUM_PATH)qtum.dylib -o main

main.o: main.cpp $(QTUM_PATH)/qtum.hpp
	$(CC) -Wall -g -c -I../../ -std=c++14 main.cpp

lldb: main
	# /usr/bin/lldb does not forward env variable
	# https://stackoverflow.com/questions/33587130/why-doesnt-lldb-forward-my-environment-variable-anymore
	DYLD_LIBRARY_PATH=${QTUM_PATH} /Applications/Xcode.app/Contents/Developer/usr/bin/lldb ./main
