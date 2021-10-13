test_syscall:
	gcc -o dummy_syscall.o dummy_syscall.c
test_tag_sys:
	gcc -pthread -o test_tag.o test_tag.c
test_func:
	gcc -I ./ -DTEST_FUNC -o test_func.o test_func.c ../utils/bitmask/bitmask.c backup_hash/hashmap.c ../utils/include/common.h
clean:
	rm *.o || true