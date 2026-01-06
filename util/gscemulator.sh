#!/usr/bin/env bash
# Simple GSCEmulator script wrapper to make setup and usage simpler
SCRIPT_DIR=$(dirname "$0")
SCRIPT_DIR=${SCRIPT_DIR:-"."}/.. # Go to the repo root

function check_deps() {
    local deps="realpath python3 pip git"

    for dep in $deps; do
        if ! which $dep >/dev/null; then
            echo "Missing required dependency: $dep"
            exit 1
        fi
    done
}

function main() {
    check_deps

    if ! [ -f "$SCRIPT_DIR/test/gscemulator/main.py" ]; then
        # Clone the repo if it wasn't already cloned and setup the venv
        rm -rf "$SCRIPT_DIR"/test/gscemulator
        mkdir "$SCRIPT_DIR"/test/gscemulator
        git clone https://github.com/HavenOverflow/gscemulator.git -b v2
        cd "$SCRIPT_DIR"/test/gscemulator

        bash setup.sh
        source venv/bin/activate
        pip install -r requirements.txt
    else
        cd "$SCRIPT_DIR"/test/gscemulator
        source venv/bin/activate
    fi

    python3 main.py $@
}

if [ "$0" == "$BASH_SOURCE" ]; then
    main $@
fi