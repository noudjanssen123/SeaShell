#!/bin/bash

gnome-terminal -- bash -c "scripts/emulate.sh"
gnome-terminal -- bash -c "build/basic /tmp/ttyV0"
gnome-terminal -- bash -c "picocom -q /tmp/ttyV1"