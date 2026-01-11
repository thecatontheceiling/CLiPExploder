CC = clang
CFLAGS = -Wall -Wextra -municode
SOURCES = main.c config.c iat_hook.c mdl_hook.c patch.c utils.c

all: release debug relsym

release:
	$(CC) -O2 $(CFLAGS) -flto -o clipexploder.exe $(SOURCES)
	strip clipexploder.exe

debug:
	$(CC) -O0 -g $(CFLAGS) -o clipexploder_debug.exe $(SOURCES)

relsym:
	$(CC) -O2 -g $(CFLAGS) -o clipexploder_relsym.exe $(SOURCES)

clean:
	rm -f clipexploder.exe clipexploder_debug.exe clipexploder_relsym.exe

.PHONY: all release debug relsym clean