PATH := $(PATH):$(CURDIR)/bin
export PATH

#all: sorted_array.o vector.o skvmap.o bintras_bst.o tsbst bintras_sort.o
# tssa

all: bintras_sort.o tsqs


CFLAGS = \
	-O2 -Wall -Wextra -fno-strict-aliasing -fno-exceptions# -D_DEBUG -g # -fsanitize=address
#	-O0
#	-Wno-maybe-uninitialized \
#	-Wno-unused-function
INCLUDE = -Iinclude -Isrc

sorted_array.o: src/sorted_array.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(INCLUDE)

vector.o: src/vector.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(INCLUDE)

skvmap.o: src/skvmap.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(INCLUDE)

bintras_bst.o: src/bintras_bst.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(INCLUDE)

bintras_sort.o: src/bintras_sort.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(INCLUDE) -std=c23

tsbst: build/bintras_bst.o tests/tsbst.c build/vector.o | bin
	cc $^ -o bin/$@ $(CFLAGS) $(INCLUDE)
	export PATH="$$PATH:$$(pwd)/bin"

tssa: tests/tssa.c build/sorted_array.o build/vector.o build/skvmap.o | bin
	cc $^ -o bin/$@ $(CFLAGS) $(INCLUDE) -Llib -lgcc_s -lgcc -lstdc++
	export PATH="$$PATH:$$(pwd)/bin"

tsqs: tests/tsqs.c build/bintras_sort.o build/vector.o | bin
	cc $^ -o bin/$@ $(CFLAGS) $(INCLUDE) -std=c23
	export PATH="$$PATH:$$(pwd)/bin"

