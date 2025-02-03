default: giftcardreader asan ubsan

giftcardreader: giftcardreader.c giftcard.h
	gcc -g -o giftcardreader.original giftcardreader.c

asan: giftcardreader.c giftcard.h
	gcc -fsanitize=address -g -o giftcardreader.asan giftcardreader.c

ubsan: giftcardreader.c giftcard.h
	gcc -fsanitize=undefined -g -o giftcardreader.ubsan giftcardreader.c

test: giftcardreader
	./runtests.sh

# .PHONY tells make to always assume this target needs
# to be rebuilt
.PHONY: clean test container-test

container-test: giftcardreader
	docker build -t giftcard-tester .
	docker run giftcard-tester

# Update clean target to remove all artifacts
clean:
	rm -f *.o giftcardreader
