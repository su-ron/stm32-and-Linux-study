cmd_/home/su/Linux_study/8_rw/hello.ko := ld -r -m elf_x86_64 -z max-page-size=0x200000 -z noexecstack  -T ./scripts/module-common.lds --build-id  -o /home/su/Linux_study/8_rw/hello.ko /home/su/Linux_study/8_rw/hello.o /home/su/Linux_study/8_rw/hello.mod.o ;  true
