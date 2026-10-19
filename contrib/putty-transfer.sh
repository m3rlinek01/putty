# Author: m3rlinek.
# Source this file in an interactive Bash or Zsh on the SSH server.
# This helper is part of the unofficial PuTTY C++ port. See ../LICENCE.

if [ -z "${BASH_VERSION-}${ZSH_VERSION-}" ]; then
    printf '%s\n' 'putty-transfer: only Bash and Zsh are supported' >&2
    return 1
fi

# The marker is deliberately not a directory. Only the cp function below
# interprets it; command cp bypasses the function and keeps ordinary cp semantics.
win=__PUTTY_WINDOWS_DOWNLOAD__
export win

_putty_transfer_emit() {
    local hex LC_ALL=C
    case "$2" in
        *[[:cntrl:]]*)
            printf '%s\n' 'putty-transfer: control characters in paths are not supported' >&2
            return 1 ;;
    esac
    hex=$(printf '%s' "$2" | LC_ALL=C od -An -v -tx1 | tr -d ' \n') || return
    # OSC has a 2048-byte payload limit. Leave room for the verb and prefix.
    if [ "${#hex}" -gt 1900 ]; then
        printf '%s\n' 'putty-transfer: path exceeds the transfer protocol limit' >&2
        return 1
    fi
    printf '\033]777;putty-transfer;%s;%s\007' "$1" "$hex"
}

_putty_transfer_cwd() {
    if ! _putty_transfer_emit cwd "$PWD"; then
        # Clear the last valid upload destination when this cwd is invalid.
        printf '\033]777;putty-transfer;cwd;\007'
        return 1
    fi
}

function cp {
    local last arg parse_options source_count recursive source_path
    local -a source_paths
    source_paths=()
    last=
    for arg do last=$arg; done
    if [ "$last" != "__PUTTY_WINDOWS_DOWNLOAD__" ]; then
        command cp "$@"
        return $?
    fi
    if [ "$#" -lt 2 ]; then
        printf '%s\n' 'putty-transfer: cp needs a source before $win' >&2
        return 2
    fi
    # Validate the entire command before emitting any requests. Keep normal cp
    # unchanged; the special destination supports only these explicit options.
    parse_options=yes
    recursive=no
    source_count=0
    for arg do
        if [ "$parse_options" = yes ]; then
            case "$arg" in
                --) parse_options=no; continue ;;
                -r|-R) recursive=yes; continue ;;
                -i) continue ;;
                -*) printf 'putty-transfer: unsupported download option: %s\n' "$arg" >&2; return 2 ;;
            esac
        fi
        [ "$arg" = "__PUTTY_WINDOWS_DOWNLOAD__" ] && continue
        source_count=$((source_count + 1))
        if [ "$source_count" -gt 16 ]; then
            printf '%s\n' 'putty-transfer: at most 16 sources per command are supported' >&2
            return 2
        fi
        _putty_transfer_emit check "$arg" >/dev/null || return
        # A slash must not hide that the source itself is a symbolic link.
        while [ "$arg" != / ] && [ "${arg%/}" != "$arg" ]; do
            arg=${arg%/}
        done
        if [ ! -e "$arg" ]; then
            printf 'putty-transfer: source does not exist: %s\n' "$arg" >&2
            return 1
        fi
        if [ -L "$arg" ]; then
            printf 'putty-transfer: symbolic links are not supported: %s\n' "$arg" >&2
            return 1
        fi
        if [ ! -f "$arg" ] && [ ! -d "$arg" ]; then
            printf 'putty-transfer: source must be a regular file or directory: %s\n' "$arg" >&2
            return 1
        fi
        if [ -d "$arg" ] && [ "$recursive" != yes ]; then
            printf 'putty-transfer: use cp -r for a directory: %s\n' "$arg" >&2
            return 1
        fi
        if [ -d "$arg" ]; then
            # Use a subshell so cd never changes the interactive shell. The
            # final slash preserves newline bytes for subsequent validation.
            source_path=$(builtin cd -P -- "$arg" >/dev/null && printf '%s/' "$PWD") || return
            source_path=${source_path%/}
            if [ "$source_path" = / ]; then
                printf '%s\n' 'putty-transfer: downloading the filesystem root is not supported' >&2
                return 1
            fi
        else
            case "$arg" in /*) source_path=$arg ;; *) source_path=$PWD/$arg ;; esac
        fi
        # Validate the byte limit, not the locale-dependent character count.
        _putty_transfer_emit check "$source_path" >/dev/null || return
        source_paths+=("$source_path")
    done
    [ "$source_count" -gt 0 ] || return 2

    for source_path in "${source_paths[@]}"; do
        _putty_transfer_emit download "$source_path" || return
    done
    # This is submission, not completion: PuTTY shows the transfer result.
}

# Install only once, preserving the user's other prompt hooks. A Bash scalar
# PROMPT_COMMAND becomes a one-element array, retaining its original command.
if [ "${_PUTTY_TRANSFER_HOOK_INSTALLED-}" != yes ]; then
    if [ -n "${BASH_VERSION-}" ]; then
        PROMPT_COMMAND=("${PROMPT_COMMAND[@]}" _putty_transfer_cwd)
    else
        precmd_functions+=(_putty_transfer_cwd)
    fi
    _PUTTY_TRANSFER_HOOK_INSTALLED=yes
fi
_putty_transfer_cwd
