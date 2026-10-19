# Duplicate Session

Log in to an SSH-2 server, then select **Duplicate Session** from the window
menu. The new window opens a separate shell as the same user over the existing
authenticated connection. Passwords are not stored or replayed. A duplicate can
also be duplicated.

Each shell has its own current directory and prompt reporting. Bash setup and
file transfers remain available. Duplication does not copy shell variables,
process memory or running programs from the original shell.

The option **Connection > SSH > Duplicate logged-in sessions without
authenticating again** is enabled by default and works with file transfers
disabled. Reconnect after changing it. Windows opened by an older executable
need a new connection before this behavior is available.

The original window owns the shared SSH connection. Closing it disconnects its
duplicates; PuTTY warns about connected downstreams. Closing a duplicate leaves
the other shells open. If the original connection disappears while a duplicate
is opening, the new window reports an error rather than making a fresh
connection or requesting a password.

An unauthenticated window, a closed connection, a disabled option or a protocol
other than standard SSH-2 uses the normal new-connection behavior.

Automated checks cover configuration copying and serialization, nested
duplicates and extra SSH channels without new TCP connections or authentication
attempts. The menu interaction itself has not been tested automatically.
