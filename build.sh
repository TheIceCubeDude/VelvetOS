echo Assembling bootloader
nasm bootloader/main.asm -f bin -o build/bootloader.bin

echo Assembling kernel
nasm kernel/core/main.asm -f elf -o build/kernel/main.o

echo Compiling kernel
# Every kernel module should contain [kernel_module.c], containing includes for all its files.
# Every kernel module should contain [include.h], containing all its function prototypes.
# Every [kernel_module.c] should include the core module's [include.h] - [../core/include.h].
# Every [kernel_module.c] should include its own [include.h], which is useful if it uses assembly routines defined in a c prototype.
i686-elf-gcc -c -ffreestanding -fPIE -mgeneral-regs-only -o build/kernel/core.o kernel/core/core.c
i686-elf-gcc -c -ffreestanding -fPIE -o build/kernel/plt_funcs.o kernel/core/plt_funcs.c
i686-elf-gcc -c -ffreestanding -fPIE -o build/kernel/video.o kernel/video/video.c
i686-elf-gcc -c -ffreestanding -fPIE -mgeneral-regs-only -o build/kernel/pit.o kernel/pit/pit.c
i686-elf-gcc -c -ffreestanding -fPIE -mgeneral-regs-only -Wno-multichar -o build/kernel/ps2.o kernel/ps2/ps2.c
i686-elf-gcc -c -ffreestanding -fPIE -o build/kernel/disk.o kernel/disk/disk.c

echo Linking kernel
i686-elf-gcc -T script.ld -nostdlib -lgcc -o build/kernel.elf build/kernel/main.o build/kernel/core.o build/kernel/plt_funcs.o build/kernel/video.o build/kernel/pit.o build/kernel/ps2.o build/kernel/disk.o

echo Compiling, assembling and linking OS programs and devel libs
#nasm -f elf -o OS_programs/libsyscall.a OS_programs/libsyscall.asm
#i686-elf-gcc -c -ffreestanding -fPIE -mgeneral-regs-only -o build/OS_programs/genesis.o OS_programs/genesis.c
#i686-elf-gcc -T script.ld -nostdlib -lgcc -LOS_programs -lsyscall -o build/OS_programs/genesis.elf build/OS_programs/genesis.o
nasm -f bin -o '|/genesis.ebin'  'OS_programs/genesis.asm'
nasm -f bin -o '|/tetrisTheme.ebin'  'OS_programs/tetrisTheme.asm'
#i686-elf-objcopy -O binary -j .text -j .data -j .rodata -j .bss -j .plt build/OS_programs/genesis.elf '|/genesis.ebin'

echo Creating rootFS
tar -H ustar -cf build/rootfs.tar '|'

echo Creating final image
i686-elf-objcopy -O binary build/kernel.elf build/kernel.bin
truncate -s 1M build/kernel.bin
truncate -s 1M build/rootfs.tar
cat build/bootloader.bin build/kernel.bin build/rootfs.tar > OS.img

echo Starting QEMU
qemu-system-i386 -hda OS.img -m 512 -audiodev id=alsa,driver=alsa -machine pcspk-audiodev=alsa
