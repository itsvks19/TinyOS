# TinyOS

## First create an disk space for OS to read and write 

```bash
dd if=/dev/zero of=disk.img bs=1M count=64
```

```bash
make
make run
```
