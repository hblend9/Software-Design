main: sort.c comparator.c main.c
ifneq ($(OS), Windows_NT)
	clang -g -fno-omit-frame-pointer -fsanitize=address -Wno-nullability-completeness sort.c comparator.c main.c -o main
else
	cl.exe -Zi -W3 -fsanitize=address -D_CRT_SECURE_NO_WARNINGS sort.c comparator.c main.c -o main.exe
endif

clean:
	rm -f main
