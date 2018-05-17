QTUM_PATH = ../../
INCLUDES = -I$(QTUM_PATH)

main: main.o
	cc main.o $(QTUM_PATH)qtum.dylib -o main

main.o: main.c
	cc -g -c -o main.o main.c $(INCLUDES)

.PHONY: init lldb

init: main
	DYLD_LIBRARY_PATH=$(QTUM_PATH) ./main init context-init.json

lldb: main
	# /usr/bin/lldb does not forward env variable
	# https://stackoverflow.com/questions/33587130/why-doesnt-lldb-forward-my-environment-variable-anymore
	DYLD_LIBRARY_PATH=${QTUM_PATH} /Applications/Xcode.app/Contents/Developer/usr/bin/lldb ./main
