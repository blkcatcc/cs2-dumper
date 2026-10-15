# linux-dumper

An external offset, interface and schema dumper for Counter-Strike 2 on Linux,
written in C.

It reads the running `cs2` process through `/proc` and writes what it finds —
offsets, interfaces, buttons, schema classes and enums, and class vtables — as
`cs`, `hpp`, `json`, `rs` and `zig`.

## References

- [Broshan1337/cs2dumper-for-linux](https://github.com/Broshan1337/cs2dumper-for-linux)
- [a2x/cs2-dumper](https://github.com/a2x/cs2-dumper)

## Build

    make

## Usage

Start CS2 and get to the main menu; that is enough, you do not need to be in a
match.

    ./linux-dumper -vv

Same-user access is all it needs. If `/proc/sys/kernel/yama/ptrace_scope` is `1`
or higher, run it with `sudo`.

## Options

| Flag | Meaning |
| --- | --- |
| `-o, --output DIR` | Output directory. Default `output`. |
| `-i, --indent-size N` | Spaces per indentation level. Default `4`. |
| `-f, --file-types LIST` | File types to write: `cs,hpp,json,rs,zig`. |
| `-p, --process-name NAME` | Process name. Default is `executable` in `config.json`. |
| `-n, --no-log-file` | Do not write `linux-dumper.log`. |
| `-v, --verbose` | Increase verbosity. Repeatable. |
| `--selfcheck` | Check every signature against an install without attaching. |
| `--game-dir DIR` | Game directory for `--selfcheck`. |
| `--selftest` | Check the output format against a fixture. No game needed. |

## Signatures

`config.json` lists the signatures. Each names a module, a byte pattern and the
operations applied in order to turn the match into a module-relative offset:
`rip` resolves a RIP-relative displacement, `read` dereferences the current
address, `add`/`sub` adjust it, and `slice` reads a byte range as a
little-endian integer.

After a game update, `--selfcheck` reports what still matches and what moved,
without starting the game.

## Output

`output/` holds the generated files plus `info.json` with the build number and a
timestamp. Module names are slugified, so `libclient.so` becomes
`libclient_so.*`. Vtables are written to `vtables.json`.
