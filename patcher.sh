#!/usr/bin/env bash
set +x

SCRIPT_DIR=$(dirname "$0")
SCRIPT_DIR=${SCRIPT_DIR:-"."}
. "${SCRIPT_DIR}/lib/common_minimal.sh"
. "${SCRIPT_DIR}/util/common.sh"

function check_deps() {
	# These are all the dependencies we'll need on the host
	local deps="gcc ldd cgpt futility python3 growpart sfdisk"
	for dep in $deps; do
		if ! which $dep >/dev/null; then
			panic "Missing required build dependency '$dep'."
		fi
	done

	# We need all of our utilities, firmware, and an updated GSCTool for our images.
	# We update GSCTool here because shims are based on the first MP images for the target board, which means
	# it's usually very outdated and buggy.
	STATIC=1 ARCH=$FLAGS_arch make utils firmware --no-print-directory
	if [ $? -ne 0 ]; then
		panic "Failed to build required dependencies."
	fi
	
	# Everything here should be modifiable without root, so let's change ownership really quick.
	chmod -R +r "${SCRIPT_DIR}"/build/* --recursive
	chown -R 1000 "${SCRIPT_DIR}"/build/* --recursive
}

function disable_verity() {
	if ! find_ssd_util; then
		panic "Failed to find CrOS SSD utility script!"
	fi

	if fbool update; then
		info "Updating current image, skipping write lock removal."
		return 0
	fi

	info "Disabling RootFS write lock."

	suppress bash "$SSD_UTIL" -i "$loop" --remove_rootfs_verification --partitions 2 --no_resign_kernel
	suppress bash "$SSD_UTIL" -i "$loop" --remove_rootfs_verification --parititons 4 --no_resign_kernel

	# We should sync a few times after doing this just to be safe, linux can be very finicky at times
	sync;sync;sync;sync
	sleep 0.2
}

function patch_shim_root() {
	local root=$(mktemp -d)

	disable_verity
	mount "${loop}p3" "$root"

	# Copy the smiko installer as the factory installer.
	info "Copying scripts to image."
	cp "${SCRIPT_DIR}/util/installer.sh" "$root/usr/sbin/factory_install.sh"
	chmod +rx "$root/usr/sbin/factory_install.sh"
	cp "${SCRIPT_DIR}/util/common.sh" "$root/usr/sbin/smiko_common.sh"
	chmod +rx "$root/usr/sbin/smiko_common.sh"

	# Place our binaries in /usr/sbin.
	info "Copying compiled binaries to image."
	local BDIR="${SCRIPT_DIR}/build/bin/$FLAGS_arch"
	cp "$BDIR/smiko" "$root/usr/sbin/smiko"
	chmod +rx "$root/usr/sbin/smiko"
	cp "$BDIR/shaft" "$root/usr/sbin/shaft"
	chmod +rx "$root/usr/sbin/shaft"
	cp "$BDIR/rmasmoke" "$root/usr/sbin/rmasmoke"
	chmod +rx "$root/usr/sbin/rmasmoke"
	cp "$BDIR/gsctool" "$root/usr/sbin/gsctool"
	chmod +rx "$root/usr/sbin/gsctool"

	# Place stock firmware in /usr/share and /opt/google/[cr/ti]50/firmware.
	info "Copying firmware to image."
	mkdir -p "$root/usr/share/smiko/loaders" \
		"$root/usr/share/smiko/firmware"
	cp "${SCRIPT_DIR}/build/firmware"/* "$root/usr/share/smiko/firmware/"
	rm -rf "$root/opt/google/cr50" "$root/opt/google/ti50"

	local firmware_path=
	for bin in "$SCRIPT_DIR"/src/cr50/prebuilts/production_images/*; do
		if echo "$bin" | grep -q "ti50" || echo "$bin" | grep -q "opentitan"; then
			firmware_path=/opt/google/ti50/firmware
		elif echo "$bin" | grep -q "cr50"; then
			firmware_path=/opt/google/cr50/firmware
		else
			# We don't need any images that aren't Cr50 or Ti50.
			continue
		fi
		mkdir -p "$root$firmware_path"
		cp "$bin" "$root$firmware_path/$(basename $bin)"
	done

	# Finally, copy any existing passwords if they exist.
	if [ -f "${SCRIPT_DIR}/local_tpm_data" ]; then
		info "Copying local TPM data."
		mkdir -p "$root/var/lib/tpm_manager"
		cp "${SCRIPT_DIR}/local_tpm_data" "$root/var/lib/tpm_manager/local_tpm_data"
		chmod +r "$root/var/lib/tpm_manager/local_tpm_data"
	fi

	sync
	sleep 1
	umount "$root"
	rmdir "$root" # I don't wanna use rm -rf because this is running as root
}

function patch_sh1mmer() {
	local targ=$(cgpt find "$loop" -l "SH1MMER")
	local size=$(printf '%d' $(sfdisk -l "$targ" | grep -o "[0-9]* bytes" | head -n 1 | sed "s/ bytes//"))
	
	# We'll want as much space as possible since the static binaries are typically large.
	if [ $size -lt 104857600 ] && ! fbool update; then 
		info "Expanding image by 64M"

		# USB devices report their full storage capacity, so growpart will expand it to fill the entire disk.
		if ! (echo "$FLAGS_image" | grep -q "/dev"); then
			dd if=/dev/zero status=none bs=16M count=4 >>"$FLAGS_image"
			# We have to redo the loop device after this because linux won't recognize the changes otherwise
			losetup -d "$loop"
			losetup -P "$loop" "$FLAGS_image"
			echo "w" | suppress fdisk "$loop" # Prevent the GPT table from corrupting
			sync # just in case
			sleep 1
		fi

		suppress growpart "$loop" 1
		#suppress e2fsck -p "$targ"
		suppress resize2fs "$targ"
		sync # one more for good measure
	fi

	local sh1mmer=$(mktemp -d)
	mount "$targ" "$sh1mmer"

	# Copy the smiko installer as a Sh1mmer payload.
	info "Copying scripts to image."
	cp "${SCRIPT_DIR}/util/installer.sh" "$sh1mmer/root/noarch/payloads/smiko.sh"
	chmod +rx "$sh1mmer/root/noarch/payloads/smiko.sh"
	cp "${SCRIPT_DIR}/util/common.sh" "$sh1mmer/root/noarch/usr/sbin/smiko_common.sh"
	chmod +rx "$sh1mmer/root/noarch/usr/sbin/smiko_common.sh"

	# Place our binaries in /usr/sbin.
	info "Copying compiled binaries to image."
	local BDIR="${SCRIPT_DIR}/build/bin/$FLAGS_arch"
	mkdir -p "$sh1mmer/root/$FLAGS_arch/usr/sbin" "$sh1mmer/root/$FLAGS_arch/usr/bin"
	cp "$BDIR/smiko" "$sh1mmer/root/$FLAGS_arch/usr/sbin/smiko"
	chmod +rx "$sh1mmer/root/$FLAGS_arch/usr/sbin/smiko"
	cp "$BDIR/shaft" "$sh1mmer/root/$FLAGS_arch/usr/sbin/shaft"
	chmod +rx "$sh1mmer/root/$FLAGS_arch/usr/sbin/shaft"
	cp "$BDIR/rmasmoke" "$sh1mmer/root/$FLAGS_arch/usr/sbin/rmasmoke"
	chmod +rx "$sh1mmer/root/$FLAGS_arch/usr/sbin/rmasmoke"
	cp "$BDIR/gsctool" "$sh1mmer/root/$FLAGS_arch/usr/sbin/gsctool"
	chmod +rx "$sh1mmer/root/$FLAGS_arch/usr/sbin/gsctool"

	# Place our firmware in /usr/share and /opt/google/[cr/ti]50/firmware.
	info "Copying firmware to image."
	mkdir -p "$sh1mmer/root/noarch/usr/share/smiko/loaders" \
		"$sh1mmer/root/noarch/usr/share/smiko/firmware"
	cp "${SCRIPT_DIR}/build/firmware"/* "$sh1mmer/root/noarch/usr/share/smiko/firmware/"
	# I'm removing all of the GSC folders, not just the firmware here to get rid of AP RO hashes
	rm -rf "$root/root/noarch/opt/google/cr50" "$sh1mmer/root/noarch/opt/google/ti50"
	local firmware_path=
	for bin in "$SCRIPT_DIR"/src/cr50/prebuilts/production_images/*; do
		if echo "$bin" | grep -q "ti50" || echo "$bin" | grep -q "opentitan"; then
			firmware_path=/opt/google/ti50/firmware
		elif echo "$bin" | grep -q "cr50"; then
			firmware_path=/opt/google/cr50/firmware
		else
			# We don't need any images that aren't Cr50 or Ti50.
			continue
		fi
		
		mkdir -p "$sh1mmer/root/noarch$firmware_path"
		touch "$sh1mmer/root/noarch$firmware_path/gscvd" # Make Sh1mmer delete the AP RO hashes
		cp "$bin" "$sh1mmer/root/noarch$firmware_path/$(basename $bin)"
	done

	# Finally, copy any existing passwords if they exist.
	if [ -f "${SCRIPT_DIR}/local_tpm_data" ]; then
		info "Copying local TPM data."
		mkdir -p "$sh1mmer/root/noarch/var/lib/tpm_manager"
		cp "${SCRIPT_DIR}/local_tpm_data" "$sh1mmer/root/noarch/var/lib/tpm_manager/local_tpm_data"
		chmod +r "$sh1mmer/root/noarch/var/lib/tpm_manager/local_tpm_data"
	fi

	sync
	sleep 1
	umount "$sh1mmer"
	rmdir "$sh1mmer" # I don't wanna use rm -rf because this is running as root
}

function loadopts() {
	load_shflags

	FLAGS_HELP="USAGE: $0 -i /path/to/image.bin [flags]"

	DEFINE_string image "" "Specify the path to the RMA Shim that will be patched to serve as the installer" "i"
	DEFINE_string arch "x86_64" "Override the architecture for the target image (Supported: x86_64, aarch64, armv7l)" "a"
	DEFINE_string out "" "Specify the path to place the patched RMA Shim Image" "o"

	DEFINE_boolean update "$FLAGS_FALSE" "Update an existing image" "u"

	FLAGS "$@" || exit $?
	eval set -- "$FLAGS_ARGV"
}

function cleanup() {
	info "Syncing disks and cleaning up."
	sync
	if ! (echo "$FLAGS_image" | grep -q "/dev"); then
		losetup -d "$loop"
	fi

	info "Patched $FLAGS_image successfully!"
	exit 0
}

function main() {
	if [ ! -f "$FLAGS_image" ] && [ ! -b "$FLAGS_image" ]; then
		panic "'$FLAGS_image' is not a real file!"
	fi

	info "Checking for unmet dependencies."
	check_deps

	if echo "$FLAGS_image" | grep -q "/dev"; then
		info "Skipping loop attachment for block device."
		loop=$FLAGS_image # Simple hack, if it works it works
	else
		info "Attaching loop device to image."
		loop=$(losetup -f)
		if [ $? -ne 0 ]; then
			panic "Failed to find an unused loop device!"
		fi

		losetup -P "$loop" "$FLAGS_image"

		if [ $? -ne 0 ]; then
			panic "Failed to loop over image, make sure it's not corrupted."
		fi
	fi



	# If we found a SH1MMER partition, this image already has Sh1mmer on it.
	if cgpt find "$loop" -l "SH1MMER" >/dev/null; then
		info "Patching Sh1mmer image with Smiko payloads."
		patch_sh1mmer
	else
		info "Patching RMA Shim with with Smiko payloads"
		patch_shim_root
	fi

	cleanup
}

if [ "$0" == "$BASH_SOURCE" ]; then
	if [ $EUID -ne 0 ]; then
		panic "Please run this script as root."
	fi

	loadopts "$@"
	main "$@"
fi