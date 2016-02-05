qemu-system-arm -M vexpress-a9 -cpu cortex-a9 \
-kernel ${MYKERNEL_PATH} \
-initrd ${MYINITRD_PATH} \
-sd ~/vexpress/vexpress-a9-nano.img \
-serial stdio -m 1024 \
-append 'root=/dev/mmcblk0p2 rw mem=1024M raid=noautodetect console=ttyAMA0,38400n8 rootwait vmalloc=256MB devtmpfs.mount=0' \
-smp 4
