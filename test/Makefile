path := "/sys/module/TAGMOD/parameters"
	
tag_get_val := $(shell cat $(path)/tag_get_nr)
tag_send_val := $(shell cat $(path)/tag_send_nr)
tag_receive_val := $(shell cat $(path)/tag_receive_nr)
tag_ctl_val := $(shell cat $(path)/tag_ctl_nr)

test_syscall:
	gcc -o dummy_syscall.o dummy_syscall.c
test_tag_sys:
	gcc -pthread -DTAG_GET_NR=$(tag_get_val) -DTAG_SEND_NR=$(tag_send_val) -DTAG_RECEIVE_NR=$(tag_receive_val) -DTAG_CTL_NR=$(tag_ctl_val) -o test_tag.o test_tag.c
	gcc -o test_char_dev.o test_char_dev.c
test_func:
	gcc -I ./ -DTEST_FUNC -o test_func.o test_func.c ../utils/bitmask/bitmask.c backup_hash/hashmap.c ../utils/include/common.h
clean:
	rm *.o || true