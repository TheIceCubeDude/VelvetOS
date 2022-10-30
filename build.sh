nasm bootloader/main.asm -f bin -o build/bootloader.bin
nasm kernel/main.asm -f bin -o build/kernel.bin 

cat build/bootloader.bin build/kernel.bin> OS.img
qemu-system-x86_64 -hda OS.img -m 512
