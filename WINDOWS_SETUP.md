# Running PIE on Windows

PIE uses Linux-specific features (perf, hardware counters) that aren't directly available on Windows. Here are your options:

---

## ✅ **Option 1: WSL2 (Recommended)**

### **What is WSL2?**
Windows Subsystem for Linux 2 - a full Linux kernel running inside Windows. Best balance of convenience and functionality.

### **Setup Steps**

#### 1. Install WSL2

```powershell
# Open PowerShell as Administrator
wsl --install -d Ubuntu-22.04

# This will:
# - Enable WSL2 feature
# - Install Ubuntu 22.04
# - Reboot (if needed)

# After reboot, set up Ubuntu username/password
```

#### 2. Update Ubuntu

```bash
# Inside WSL2 Ubuntu terminal
sudo apt update
sudo apt upgrade -y
```

#### 3. Install PIE Dependencies

```bash
# Install build tools
sudo apt install -y \
    cmake \
    build-essential \
    clang-14 \
    llvm-14-dev \
    python3 \
    python3-pip \
    git

# Note: perf support in WSL2 is limited
# Install anyway (some features may not work)
sudo apt install linux-tools-generic

# Install Python packages
pip3 install flask flask-cors numpy pandas matplotlib psutil pyyaml
```

#### 4. Clone and Build PIE

```bash
# Clone (or access your Windows filesystem)
# Your Windows drives are at /mnt/c, /mnt/d, etc.
cd /mnt/y/PIE

# Or clone fresh
# git clone <your-repo> ~/PIE
# cd ~/PIE

# Build
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### 5. Test It

```bash
# Check what works
./pie info

# Try workloads (without full perf support)
./pie profile matmul --size 256
```

### **⚠️ WSL2 Limitations**

**What Works:**
- ✅ Building the project
- ✅ Running workloads
- ✅ Basic timing measurements
- ✅ LLVM optimization passes
- ✅ Dashboard (access at http://localhost:5000)
- ✅ GPU (if you have WSL2 GPU support enabled)

**What's Limited:**
- ⚠️ **Hardware counters** (perf) - Limited or unavailable
  - WSL2 doesn't expose full PMU access
  - You'll get timing data but not detailed cache/branch stats
  
**Workaround**: The project will still run and show timing improvements, just without detailed hardware metrics.

### **Accessing Files**

```bash
# Your Windows files are accessible in WSL2:
# C:\ is at /mnt/c/
# Y:\ is at /mnt/y/

# So your PIE project at Y:\PIE is:
cd /mnt/y/PIE

# Edit files in Windows, build in WSL2!
```

### **Using VS Code with WSL2**

```powershell
# Install VS Code extension: "Remote - WSL"
# Then in WSL2 terminal:
cd /mnt/y/PIE
code .

# VS Code will open with full WSL2 integration!
```

---

## 🐳 **Option 2: Docker Desktop**

### **Setup**

1. Install [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop/)

2. Create Dockerfile:

```dockerfile
# Save this as Dockerfile in Y:\PIE\
FROM ubuntu:22.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    clang-14 \
    llvm-14-dev \
    python3 \
    python3-pip \
    linux-tools-generic \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip3 install flask flask-cors numpy pandas matplotlib psutil pyyaml

# Set working directory
WORKDIR /pie

# Copy project files
COPY . .

# Build PIE
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Default command
CMD ["/bin/bash"]
```

3. Build and run:

```powershell
# In PowerShell at Y:\PIE\
docker build -t pie .

# Run interactively
docker run -it --rm pie

# Inside container:
cd build
./pie info
./pie profile matmul --size 256
```

### **Docker Limitations**

- ⚠️ Hardware counters still limited (even with --privileged)
- ⚠️ No GPU access (unless configured)
- ✅ Isolated environment
- ✅ Easy to reproduce

---

## 🖥️ **Option 3: Virtual Machine**

### **Setup VirtualBox + Ubuntu**

1. Download [VirtualBox](https://www.virtualbox.org/)
2. Download [Ubuntu 22.04 ISO](https://ubuntu.com/download/desktop)
3. Create VM:
   - 4+ GB RAM
   - 2+ CPU cores
   - 20+ GB disk
4. Install Ubuntu
5. Follow standard Linux installation from `INSTALLATION.md`

### **Advantages**
- ✅ Full hardware counter access (if VT-x enabled)
- ✅ Complete Linux environment
- ✅ GPU passthrough possible

### **Disadvantages**
- ⚠️ Slower performance
- ⚠️ More resource-intensive
- ⚠️ More setup required

---

## ☁️ **Option 4: Cloud VM (Quick Testing)**

### **Free Options**

**Google Cloud Platform** (300$ free credit):
```bash
# Create Ubuntu VM
gcloud compute instances create pie-vm \
    --image-family=ubuntu-2204-lts \
    --image-project=ubuntu-os-cloud \
    --machine-type=n1-standard-2

# SSH into it
gcloud compute ssh pie-vm

# Install PIE (follow INSTALLATION.md)
```

**AWS EC2** (Free tier):
- Launch t2.micro Ubuntu instance
- SSH and install PIE

**Azure** (Free tier):
- Create B1s Ubuntu VM
- Install PIE

### **Advantages**
- ✅ Real Linux environment
- ✅ Full hardware counters
- ✅ No local resources used
- ✅ Can access from anywhere

---

## 🎯 **Recommended Approach for Windows Users**

### **For Quick Testing:**
```
WSL2 → Build → Test basic functionality → View timing improvements
```

### **For Full Experience:**
```
Cloud VM (GCP/AWS) → Full Linux → Real hardware counters → Complete profiling
```

### **For Development:**
```
VS Code + WSL2 → Edit in Windows → Build/Test in WSL2 → Push to GitHub
```

---

## 🚀 **Quick Start for Your Situation**

Since you're at `Y:\PIE\` on Windows:

### **Step 1: Install WSL2 (5 minutes)**

```powershell
# In PowerShell (Admin):
wsl --install -d Ubuntu-22.04
# Reboot if prompted
```

### **Step 2: Setup Ubuntu**

```bash
# After WSL2 starts, create username/password
# Then:
sudo apt update
sudo apt install -y cmake build-essential clang-14 llvm-14-dev python3 python3-pip git
```

### **Step 3: Navigate to Your Project**

```bash
# Your Y: drive is at /mnt/y in WSL2
cd /mnt/y/PIE
ls  # You'll see all your files!
```

### **Step 4: Build**

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

### **Step 5: Test**

```bash
./pie info
./pie list
./pie profile matmul --size 256

# Start dashboard (accessible from Windows browser)
cd ../dashboard
python3 server.py
# Open Chrome: http://localhost:5000
```

---

## 📊 **What You'll Get vs Linux**

| Feature | Windows (WSL2) | Native Linux |
|---------|----------------|--------------|
| **Build System** | ✅ Full | ✅ Full |
| **LLVM Passes** | ✅ Full | ✅ Full |
| **Workloads** | ✅ Full | ✅ Full |
| **Timing** | ✅ Full | ✅ Full |
| **Speedup Validation** | ✅ Full | ✅ Full |
| **Dashboard** | ✅ Full | ✅ Full |
| **Hardware Counters** | ⚠️ Limited | ✅ Full |
| **Detailed Profiling** | ⚠️ Limited | ✅ Full |
| **IPC Metrics** | ⚠️ May not work | ✅ Full |
| **Cache Miss Stats** | ⚠️ May not work | ✅ Full |

**Bottom line**: You can still demonstrate all the core concepts and see performance improvements, just without detailed hardware-level metrics.

---

## 💡 **Alternative: Demo Mode**

If you just want to show the project:

1. **Use the code** - Show the implementation quality
2. **Use the docs** - Demonstrate understanding
3. **Show the architecture** - Explain the design
4. **Reference results** - Use documented benchmarks
5. **Video demo** - Record on Linux VM/Cloud and present that

For interviews, explaining the architecture and design decisions is often more valuable than live demos anyway!

---

## 🎬 **Creating a Demo Video (Best for Presentations)**

If you have access to a Linux machine (or Cloud VM):

```bash
# Record your terminal session
sudo apt install asciinema

# Start recording
asciinema rec pie-demo.cast

# Run demo commands
./pie info
./pie analyze matmul --size 1024
./pie optimize matmul --size 1024

# Stop recording (Ctrl+D)

# Upload and share
asciinema upload pie-demo.cast
```

---

## ❓ **FAQ**

**Q: Can I get full hardware counter support on Windows?**  
A: No, not directly. WSL2 has limited PMU access. Use cloud VM or dual-boot Linux for full features.

**Q: Will optimizations still work in WSL2?**  
A: Yes! LLVM passes work fully. You'll see timing improvements, just not detailed counter breakdowns.

**Q: Can I use Windows Terminal?**  
A: Yes! Windows Terminal has great WSL2 integration. Recommended.

**Q: What about GPU support?**  
A: WSL2 supports GPU if you have Windows 11 + latest drivers (CUDA/OpenCL).

**Q: Is this good enough for demos?**  
A: Yes for showing code/architecture. For full profiling demos, use cloud VM.

---

## 🎯 **My Recommendation for You**

Based on your situation (Windows, Y:\PIE):

1. **Install WSL2** (15 minutes)
2. **Build in WSL2** (5 minutes)
3. **Test basic functionality** (see timing improvements)
4. **For interviews**: Explain the architecture (hardware counters not required)
5. **For full demo**: Spin up free GCP VM (1 hour setup)

**You don't need full hardware counters to demonstrate the project's value!**

The implementation, architecture, and understanding matter more than live profiling data.

---

## 📞 **Need Help?**

Common issues and solutions in WSL2:
- "perf not found" → Normal, install anyway: `sudo apt install linux-tools-generic`
- "Permission denied" → Use `sudo` or check WSL2 config
- "Can't find files" → Remember `/mnt/y/PIE` not `Y:\PIE`
- "Port already in use" → Windows might be using port, try different port

---

**Ready to try it? Start with WSL2 - it's the path of least resistance!** 🚀

