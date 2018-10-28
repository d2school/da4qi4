CFLAGS=-g

all: a.gif testlib

testlib: libcaptcha.a
	gcc -o test test.c libcaptcha.a
	./test | identify -

a.gif: captcha	
	./captcha | identify -


libcaptcha.a: libcaptcha.o
	ar r libcaptcha.a libcaptcha.o

captcha: libcaptcha.c
	gcc $(CFLAGS) -DCAPTCHA -o captcha libcaptcha.c

f.h: unfont
	./unfont > .f.h
	mv .f.h f.h

libcaptcha.c: captcha.c f.h
	(echo "// Version $(VERSION)" && echo '// zlib/libpng license is at the end of this file' && grep -v '^#include "f.h"' captcha.c && cat f.h && echo '/*' && cat LICENSE && echo '*/') > .libcaptcha.c
	mv .libcaptcha.c libcaptcha.c
	
clean:
	rm -f captcha libcaptcha.a libcaptcha.o libcaptcha.c test f.h .libcaptcha.c .f.h unfont core


publish: clean
	$(MAKE) VERSION="$(shell date -I) (http://github.com/ITikhonov/captcha/tree/$(shell git show-ref --verify --hash refs/heads/master))" all
