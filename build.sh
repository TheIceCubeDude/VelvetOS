nasm bootloader/main.asm -f bin -o build/bootloader.bin

cat build/bootloader.bin > OS.bin
qemu-system-x86_64 -hda OS.bin
