#!/bin/sh
# Builds if needed, then dumps the running game.
set -e

cd "$(dirname "$0")"

# Rebuild whenever a source file is newer than the binary, so a stale binary is
# never used after an edit.
if [ ! -x linux-dumper ] || [ -n "$(find src -name '*.c' -newer linux-dumper 2>/dev/null)" ] \
   || [ config.json -nt linux-dumper ]; then
    make
fi

# --selfcheck/--selftest/--help never read the game's memory, so no elevation.
case " $* " in
    *" --selfcheck "*|*" --selftest "*|*" --help "*|*" -h "*) exec ./linux-dumper "$@" ;;
esac

# Same-user /proc access needs ptrace_scope 0; otherwise elevate.
if [ "$(cat /proc/sys/kernel/yama/ptrace_scope 2>/dev/null || echo 0)" != "0" ]; then
    exec sudo ./linux-dumper "$@"
fi

exec ./linux-dumper "$@"
