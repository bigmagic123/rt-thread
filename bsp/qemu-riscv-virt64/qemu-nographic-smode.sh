qemu-system-riscv64 -nographic -machine virt -m 256M -kernel rtthread.bin -netdev user,id=net0,hostfwd=tcp::26999-:80 -object filter-dump,id=net0,netdev=net0,file=packets.pcap -device e1000,netdev=net0,bus=pcie.0
