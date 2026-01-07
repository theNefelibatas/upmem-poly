# Polynomial

## Environment Configuration

- WSL2 Ubuntu-22.04
- upmem-2023.2.0-Linux-x86_64

### UPMEM

Unzip UPMEM sdk to any path, for example `/home/<user>/upmem`.

Check if you already success:

```bash
source ~/upmem/upmem_env.sh
cd && git clone git@github.com:upmem/dpu_demo.git
cd dpu_demo && make test
```

If any error occurs, commands below may be helpful:

```bash
sudo apt update
sudo apt upgrade
sudo apt install -y build-essential pkg-config libtinfo5
sudo ln -s /usr/lib/x86_64-linux-gnu/libpython3.10.so.1.0 /usr/lib/x86_64-linux-gnu/libpython3.6m.so.1.0
```

### Other

To generate `compile_commands.json` file so we can allow `clangd` find the correct include directories, run:

```bash
sudo apt update
sudo apt upgrade
sudo apt install -y bear
bear -- make
```
