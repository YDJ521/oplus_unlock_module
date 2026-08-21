# OnePlus Ace5 Pro Kernel Module

## Features

1. **144Hz Display Overclock** - Adds 144Hz mode to display panel (native max: 120Hz)
2. **55W PPS Fast Charging Unlock** - Unlocks PPS fast charging protocol

## Device Compatibility

| Property | Value |
|----------|-------|
| Device | OnePlus Ace5 Pro |
| SoC | Snapdragon 8 Gen 3 (sm8650) |
| Kernel | 6.6.89-android15-8 |
| Root | KernelSU required |

## Build Requirements

### Option 1: Android NDK (Recommended)

1. Install Android NDK:
   ```
   winget install Google.AndroidNDK
   ```

2. Set environment variable:
   ```
   set ANDROID_NDK_HOME=C:\Users\<username>\AppData\Local\Android\Sdk\ndk\<version>
   ```

3. Build:
   ```
   cd module
   scripts\build.bat
   ```

### Option 2: LLVM for Windows

1. Install LLVM:
   ```
   winget install LLVM.LLVM
   ```

2. You also need kernel headers from the OnePlus kernel source.

### Option 3: Linux Build Environment (Recommended for production)

On a Linux machine with proper kernel headers:

```bash
# Install dependencies
sudo apt install clang llvm libelf-dev

# Set paths
export KDIR=/path/to/kernel/source
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

# Build
make -C $KDIR M=$(pwd)/module/src modules
```

## Kernel Headers

You need kernel headers for the OnePlus Ace5 Pro. These can be obtained from:

1. **OnePlus Kernel Source**: https://github.com/OnePlusOSS
2. **Extract from device**: Not available on this device
3. **Pre-built headers**: Check XDA forums for your device

Extract kernel headers to: `module/kheaders/`

The headers should contain at minimum:
- `include/linux/kprobes.h`
- `include/linux/module.h`
- `include/linux/kernel.h`
- `arch/arm64/include/`

## Installation

### KernelSU Manager

1. Build the module (see above)
2. Zip the `module/` directory contents
3. Install via KernelSU Manager > Modules > Install from storage

### Manual Installation

```bash
adb push module/oplus_unlock.ko /data/local/tmp/
adb shell su -c "insmod /data/local/tmp/oplus_unlock.ko enabled=1"
```

## Usage

### Module Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| enabled | bool | true | Master switch |
| overclock_display | bool | true | Enable 144Hz |
| unlock_pps | bool | true | Enable 55W PPS |
| observe | bool | false | Debug logging |
| hits | uint | (read-only) | Hook hit counter |

### Check if loaded

```bash
adb shell su -c "ls /sys/module/oplus_unlock/"
adb shell su -c "cat /sys/module/oplus_unlock/parameters/hits"
adb shell su -c "dmesg | grep oplus_unlock"
```

### Verify 144Hz mode

```bash
adb shell su -c "dumpsys SurfaceFlinger"
# Look for 144Hz in display modes
```

### Verify PPS charging

```bash
# Connect PPS charger
adb shell su -c "dmesg | grep pps"
adb shell su -c "cat /sys/class/power_supply/battery/current_now"
```

## Troubleshooting

### Module fails to load

1. Check vermagic matches:
   ```bash
   adb shell su -c "cat /proc/version"
   ```

2. Check dmesg for errors:
   ```bash
   adb shell su -c "dmesg | tail -50"
   ```

### System crashes after insmod

1. Boot to safe mode (hold volume down during boot)
2.卸载模块:
   ```bash
   adb shell su -c "rmmod oplus_unlock"
   ```

3. Check KCFI settings match

### No effect after loading

1. Verify parameters:
   ```bash
   adb shell su -c "cat /sys/module/oplus_unlock/parameters/enabled"
   ```

2. Enable observe mode:
   ```bash
   adb shell su -c "echo 1 > /sys/module/oplus_unlock/parameters/observe"
   ```

## Technical Details

### Display Overclock

The module hooks `dsi_connector_get_modes()` in the msm_drm driver to inject a 144Hz display mode. The mode timing is derived from the existing 120Hz mode.

**Risk**: The panel hardware may not support 144Hz. This could cause:
- Black screen
- Screen flickering
- Permanent panel damage

### PPS Unlock

The module hooks `oplus_pps_charge_allow_check()` in oplus_chg_v2 to always return 1 (allowed), bypassing the PPS power limit.

**Risk**: 
- Overheating
- Battery degradation
- Charging IC damage

## Uninstallation

```bash
# Remove module
adb shell su -c "rmmod oplus_unlock"

# Or via KernelSU Manager
# Modules > OnePlus Unlock > Uninstall
```

## License

GPL-2.0-only
