echo Assembling bootloader
nasm bootloader/main.asm -f bin -o build/bootloader.bin

echo Assembling kernel
nasm kernel/main.asm -f elf -o build/kernelMain.o
echo Compiling kernel
i686-elf-gcc -c -ffreestanding -fPIE -o build/kernel.o kernel/kernel.c

echo Linking kernel
i686-elf-gcc -T script.ld -nostdlib -lgcc -o build/kernel.elf build/kernelMain.o build/kernel.o

echo Creating final image
i686-elf-objcopy -O binary build/kernel.elf build/kernel.bin
truncate -s 1M build/kernel.bin
cat build/bootloader.bin build/kernel.bin> OS.img

echo Starting QEMU
qemu-system-x86_64 -hda OS.img -m 512
