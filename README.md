# Dragonfruit

A CLI music player built using PulseAudio and FTXUI.

## Building from source

Prebuilt packages are not available for Dragonfruit. Building the project from source is required.

1. Install necessary dependencies for building:
```bash
apt install build-essential cmake pkg-config libpulse-dev
```

2. Ensure the PulseAudio runtime library is installed:
```bash
apt install libpulse0
```

3. Clone the repository:
```bash
git clone github.com/ShinraiYeen/Dragonfruit
```

4. Build the project:
```bash
cd Dragonfruit
mkdir -p build
cd build
cmake ..
make -j8
```

## Installing
After building the project, Dragonfruit can be installed using the following:
```bash
make install
```

