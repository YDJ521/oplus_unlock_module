#!/system/bin/sh
# KernelSU module service script
# Waits for dependencies and loads the kernel module

MODDIR=${0%/*}
KMOD="$MODDIR/oplus_unlock.ko"
MODNAME="oplus_unlock"

# Load configuration if exists
[ -r "$MODDIR/profile.conf" ] && . "$MODDIR/profile.conf"

# Check if already loaded
[ -d "/sys/module/$MODNAME" ] && exit 0

# Check if module exists
[ -f "$KMOD" ] || exit 1

# Wait for kernel modules to be ready
attempt=0
while [ "$attempt" -lt 30 ]; do
    # Check if msm_drm and oplus_chg_v2 are loaded
    if [ -d /sys/module/msm_drm ] && [ -d /sys/module/oplus_chg_v2 ]; then
        # Load the module
        if insmod "$KMOD" enabled=1 observe=0; then
            log -t "$MODNAME" "loaded successfully"
            exit 0
        else
            log -t "$MODNAME" "insmod failed"
            exit 1
        fi
    fi
    attempt=$((attempt + 1))
    sleep 1
done

log -t "$MODNAME" "dependencies not ready after 30s"
exit 1
