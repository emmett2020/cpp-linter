#!/bin/sh
# This is the entrypoint of GITHUB action.

# `$#` expands to the number of arguments and `$@` expands to the supplied `args`
printf '%d args:' "$#"
printf " '%s'" "$@"
printf '\n'
/usr/local/bin/cpp-linter
