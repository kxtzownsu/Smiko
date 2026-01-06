function fbool() {
    [ "$(eval echo $(echo \"\$FLAGS_$1\"))" = "$FLAGS_TRUE" ]
}

function info() {
    echo -ne "\033[32;1mInfo: $@\033[0m\n"
    sleep 0.2
}

function warning() {
    echo -ne "\033[33;1mWarning: $@\033[0m\n"
    sleep 1.5
}

function panic() {
    echo -ne "\033[31;1mFatal Error: $@\033[0m\n"
    sleep 0.2
    exit 1
}

function debug() {
    if fbool debug; then
        echo -ne "\033[36;1mDebug: $@\033[0m\n"
        sleep 0.2
    fi
}

function wait_for_user() {
    info "Press enter to continue."
    read -p "" e
}

function runtask() {
    info "Beginning task $1."

    if $@; then
        info "Task $1 returned success."
        sleep 1
    else
        warning "Task $1 returned failure!"
        wait_for_user
    fi
}

function suppress() {
    if fbool debug; then
        $@
    else
        $@ >/dev/null 2>/dev/null
    fi
}

function twicesystem() {
    crossystem $@
    crossystem $@ # Grunt board weirdness
}

function find_ssd_util() {
    local paths="/usr/share/vboot/bin . ./image_utils ./lib"
    
    for path in $paths; do
        if [ -f "$path/ssd_util.sh" ]; then
            SSD_UTIL="$path/ssd_util.sh"
            chmod +rx "$SSD_UTIL" # Mark it as executable just in case
            return 0
        fi
    done

    panic "Failed to find required ChromeOS SSD utility!"
}

