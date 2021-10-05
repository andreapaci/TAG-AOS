test_syscall:
	gcc -o dummy_syscall.o dummy_syscall.c
test_tag_sys:
	gcc -o test_tag.o test_tag.c
test_func:
	gcc -I ./ -DTEST_FUNC -o test_func.o test_func.c ../tag-module/util/bitmask.c ../tag-module/hash-struct/hashmap.c
clean:
	rm *.o || true