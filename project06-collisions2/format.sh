#!/bin/bash

# Simple wrapper around a code formatter to filter which files get formatted.
# Author: Noah D. Ortiz <abstractednoah@brumal.org>

# Exit immediately on a non-zero exit status.
set -e

case ${1} in
    -h|--help)
        printf "%s\n" \
            "Format the project, by default with 'clang-format'," \
            "and strip trailing whitespace." \
            "Modify the script to change which files get formatted." \
            "Modify '.clang-format' to control the behavior of clang-format."
        printf \
            "usage: [FORMATTER=<executable>] [FORMATTER_ARGS=<args>] %s\n" \
            ${0}
        exit 1
        ;;
esac

# Can be overridden by environment variables.
FORMATTER=${FORMATTER:=clang-format}
FORMATTER_ARGS=${FORMATTER_ARGS:=-i --verbose}

# Add filters from first to third-to-last line inclusively to control which
# files get formatted. See 'man find' for details.
find . -type f -not \( -path './.git/*' \) \
    \( -name '*.c' -or -name '*.h' \) \
    -exec sed -i 's/\s\+$//' {} \; \
    -exec "${FORMATTER}" ${FORMATTER_ARGS} {} \;

# (Note that there should be a way to improve the exclusion of '.git', so that
# it doesn't recurse into the directory, but I don't understand 'find' well
# enough for that. There is a '-prune' flag, but I'm not sure whether it only
# works with '-type d'.)
