#!/bin/bash
# Thanks https://github.com/MercuryWorkshop/RecoMod devs for the main parts of the GUI!
# This script is designed to load a GUI for running Smiko on a ChromeOS RMA Factory Repair Shim

BOX_H="\xe2\x94\x81"
BOX_V="\xe2\x94\x83"
BOX_TR="\xe2\x94\x93"
BOX_TL="\xe2\x94\x8f"
BOX_BR="\xe2\x94\x9b"
BOX_BL="\xe2\x94\x97"

RED='\033[1;31m'
GREEN='\033[1;32m'
BLUE='\033[1;34m'
RESET='\033[0m'

splash="~ Smiko Installer ~"
version="Smiko Installer Version 1.3.0"

function readinput() {
	local CHR_ESC=$(printf "\x1b")
	local CHR_BS=$(printf "\x08")
	local CHR_DEL=$(printf "\x7f")
	local mode
	# discard stdin
	read -rsn 10000 -t 0.1 mode || :
	read -rsn1 mode

	case "$mode" in
		"$CHR_ESC") read -rsn2 mode ;;
		"$CHR_BS" | "$CHR_DEL") echo kB ;;
		"") echo kE ;;
		*) echo "$mode" ;;
	esac

	case "$mode" in
		"[A") echo kU ;;
		"[B") echo kD ;;
		"[D") echo kL ;;
		"[C") echo kR ;;
	esac
}

function repeat() {
	i=0
	while [ $i -le $2 ]; do
		echo -en "$1"
		((i++))
	done
}


function message() {
	local height=$(tput lines)
	local width=$(tput cols)
	clear

	echo -ne "\x1b[0;$(((width - $(expr length "$splash")) / 2))f"
	echo "$splash"
	
	stty -echo
	stty -icanon
	tput civis

	local len=$(expr length "$1")
	echo -ne "\x1b[$((height / 2));$(((width - len) / 2))f"

	echo "$1"

	echo -ne "\x1b[$((height - 1));2f"
	echo "$version"
}

function pick() {
	local height=$(tput lines)
	local width=$(tput cols)
	clear
	stty -isig
	stty -echo
	stty -icanon
	tput civis

	echo -ne "\x1b[0;$(((width - $(expr length "$splash")) / 2))f"
	echo "$splash"

	local tlen=$(expr length "$1")
	title=$1
	shift

	mlen=0

	for i in "$@"; do
    	len=$(expr length "$i")
		if [ $len -gt $mlen ]; then
			mlen=$len
		fi
	done

	startx=$(((width - mlen) / 2))
	starty=$(((height - $# + 1) / 2))

	echo -ne "\x1b[$((starty - 4));$(((width - tlen) / 2))f"
	echo -ne "$title"

	echo -ne "\x1b[$((starty - 2));$((startx - 3))f"
	echo -ne "$BOX_TL"
	repeat "$BOX_H" $((mlen + 8))
	echo -ne "$BOX_TR"
	repeat "\x1b[1B\x1b[1D$BOX_V" $(($# + 1))
	echo -ne "\x1b[$((starty + $# + 1));$((startx - 3))f"
	echo -ne "$BOX_BL"
	repeat "$BOX_H" $((mlen + 8))
	echo -ne "$BOX_BR"
	echo -ne "\x1b[$((starty - 2));$((startx - 2))f"
	repeat "\x1b[1B\x1b[1D$BOX_V" $(($# + 1))

	helptext="Use the arrow keys to navigate, press enter to select"
	elen=$(expr length "$helptext")
	echo -ne "\x1b[$((starty + $# + 3));$(((width - elen) / 2))f"
	echo -ne "$helptext"

	echo -ne "\x1b[$((height - 1));2f"
	echo "$version"

	selected=0
	while true; do
		idx=0
		for opt; do
			echo -ne "\x1b[$((idx + starty));${startx}f"
			if [ $idx -eq $selected ]; then
				echo -ne "${BLUE}--> $(echo $opt)${RESET}"
			else
				echo -ne "${RESET}    $(echo $opt)"
			fi
			((idx++))
		done
		input=$(readinput)
		case $input in
		'kE')
			CHOICE=$((selected + 1))
			stty echo
			setterm -cursor on
			clear
			return
			;;
		'kU')
			selected=$((selected - 1))
			if [ $selected -lt 0 ]; then selected=$(($# - 1)); fi
			;;
		'kD')
			selected=$((selected + 1))
			if [ $selected -ge $# ]; then selected=0; fi
			;;
		esac
	done
}

function stall() {
	local count=$1
	while [ $count -gt 0 ]; do
		printf "${count}... "
		sleep 1
		((count--))
	done
	
	echo ""
}



function pick_file() {
	firmwares=$(printf '%s\n' $*)
	data="$(while read line; do
		echo -n "$(basename $line) "
	done <<EOF
$firmwares
EOF
)"

	pick "Please select a firmware to install." $data
	sel=$(echo "$firmwares" | sed "${CHOICE}q;d")
}

function write_stock() {
	sleep 0.2
	pick_file /opt/google/cr50/firmware/* /opt/google/ti50/firmware/*

	clear
	smiko --headerinfo "$sel"
	smiko --verify "$sel"

	echo "Preparing to flash firmware in "
	stall 3

	# Only program the RW in stock images.
	smiko --flash "$sel" --section RW

	sleep 0.2
	read -p "Press enter to continue." e
}

function write_firmware() {
	sleep 0.2
	pick_file /usr/share/smiko/firmware/*

	clear
	echo "Preparing to flash firmware in "
	stall 3
	smiko --flash "$sel"

	sleep 0.2
	read -p "Press enter to continue." e
}

function finalize() {
	pick "Fix the image_size field and finalize installed firmware?" \
		"Cancel" \
		"Continue"
	case "$CHOICE" in
	1) return ;;
	2) smiko --finalize ;;
	esac
}
function sysinfo() {
	clear
	echo -ne "\n\n\n"
	smiko --sysinfo
	echo -ne "\n\n\n"
	read -p "Press enter to return to the previous menu." e
}
function spawn_shell() {
	clear
	setterm -cursor on
	stty echo

	bash
	local ret=$?

	stty -echo
	setterm -cursor off

	return $ret
}
function ccd_shell() {
	# G_VID, see smiko/include/usb.h
	if ! lsusb | grep -q "18d1"; then
		message "No Google USB vendors detected!"
		sleep 3
		return
	fi

	smiko --console /dev/ttyUSB0
}
function rescue() {
	if ! which shaft >/dev/null 2>/dev/null; then
		message "Shaft is not installed in this image, cannot do rescue!"
		sleep 3
		return
	fi

	pick_file /opt/google/cr50/firmware/* \
		/opt/google/ti50/firmware/* \
		/usr/share/smiko/firmware/*
	
	clear
	echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Warning !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	echo "Rescue mode is a destructive operation! Continuing will result in the target"
	echo "device's NVMEM being wiped (meaning ALL TPM2 data and more will be IRRECOVERABLY"
	echo "LOST!) and the RW banks will be completely wiped for reprogramming!"
	echo ""
	echo "For users attempting Extortion:"
	echo "Rescue cannot run CFW on devices on Cr50 RO 0.0.11 and up! Verify the target's version"
	echo "via 'smiko -i -s' or by running 'sysinfo' on the UART console! If you try to run CFW"
	echo "and the RO is too high, you'll be left with a brick!"
	echo ""
	echo "If anything goes wrong with the rescue process, the target could be left in an unusable"
	echo "state! HavenOverflow is not responsible for damage caused by you using this!"
	sleep 3
	read -p "Please type 'PROCEED' to continue> " proceed
	if [ $proceed -eq "PROCEED" ]; then
		clear
		stall 5

		shaft --rescue "$sel" /dev/ttyUSB0 --extortion --verbose
	fi
}
function bootstrap() {
	if ! which shaft >/dev/null 2>/dev/null; then
		message "Shaft is not installed in this image, cannot do rescue!"
		sleep 3
		return
	fi

	pick_file /opt/google/cr50/firmware/* \
		/opt/google/ti50/firmware/* \
		/usr/share/smiko/firmware/*
	
	clear
	echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Warning !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	echo "Bootstrap mode is a destructive operation! Continuing will result in the target"
	echo "device's NVMEM being wiped (meaning ALL TPM2 data and more will be IRRECOVERABLY"
	echo "LOST!) and the ENTIRE FLASH will be completely wiped for reprogramming!"
	echo ""
	echo "If anything goes wrong with the bootstrap process, the target could be left in an unusable"
	echo "state! HavenOverflow is not responsible for damage caused by you using this!"
	sleep 3
	read -p "Please type 'PROCEED' to continue." proceed
	if [ $proceed -eq "PROCEED" ]; then
		clear
		stall 5

		shaft --bootstrap "$sel" /dev/ttyUSB0 --verbose
	fi
}
function reboot_gui() {
	message "Exiting..."
	sync

	if is_sh1mmer_payload; then
		reboot -f
		sleep 1d
	else
		clear
		stty echo
		setterm -cursor on
		exit 0
	fi
}

function twicesystem() {
	crossystem $@
	crossystem $@ # Grunt board weirdness
}

function runtask() {
	sleep 0.2
	if $@; then
		message "Task $1 succeeded."
	else
		message "Task $1 failed!"
	fi
	message "Press enter to continue."
	read -p "" e
}

function is_sh1mmer_payload() {
	[ -d /usr/share/sh1mmer-assets ]
}

function main() {
	trap reboot_gui SIGINT
	clear
	
	CHOICE=0
	while true; do
		pick "Please select an action." \
			"Install Stock GSC Firmware" \
			"Install Custom GSC Firmware" \
			"View GSC System Information" \
			"Open Bash Shell (For debugging purposes)" \
			"Open CCD Console (Connect SuzyQ or Servo!)" \
			"Rescue a connected GSC over UART (Connect Servo!)" \
			"Bootstrap a connected GSC over SPS (Connect Servo!)" \
			"Finalize Modifications and Reboot" \
			"Exit the Smiko Installer"

		case "$CHOICE" in
		1) runtask write_stock ;;
		2) runtask write_firmware ;;
		3) runtask sysinfo ;;
		4) runtask spawn_shell ;;
		5) runtask ccd_shell ;;
		6) runtask rescue ;;
		7) runtask bootstrap ;;
		8) runtask finalize ;;
		9) reboot_gui ;;
		esac
	done

	clear
	exec bash # Failsafe, hopefully unreachable
}

if [ "$0" == "$BASH_SOURCE" ]; then
	# For host testing
	if [ $EUID -ne 0 ]; then
		echo "Please run this script as root."
		exit 1
	fi

	main "$@"
fi
