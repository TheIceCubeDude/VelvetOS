echo Assembling bootloader
nasm bootloader/main.asm -f bin -o build/bootloader.bin

echo Assembling kernel
nasm kernel/core/main.asm -f elf -o build/kernel/main.o

echo Compiling kernel
# Every kernel module should contain [kernel_module.c], containing includes for all its files.
# Every kernel module should contain [include.h], containing all its function prototypes.
# Every [kernel_module.c] should include the core module's [include.h] - [../core/include.h].
# Every [kernel_module.c] should include its own [include.h], which is useful if it uses assembly routines defined in a c prototype.
i686-elf-gcc -c -ffreestanding -fPIE -o build/kernel/core.o kernel/core/core.c
i686-elf-gcc -c -ffreestanding -fPIE -o build/kernel/video.o kernel/video/video.c

echo Linking kernel
i686-elf-gcc -T script.ld -nostdlib -lgcc -o build/kernel.elf build/kernel/main.o build/kernel/core.o build/kernel/video.o

echo Creating final image
i686-elf-objcopy -O binary build/kernel.elf build/kernel.bin
truncate -s 1M build/kernel.bin
cat build/bootloader.bin build/kernel.bin> OS.img

echo Starting QEMU
qemu-system-x86_64 -hda OS.img -m 512
