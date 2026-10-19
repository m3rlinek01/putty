# Planned work

These changes have not been implemented.

1. Reduce C++ compatibility warnings one utility module at a time. Start with
   const correctness and explicit conversions, using the module's existing tests.
2. Replace Clang-specific C99 initializers in small groups. Preserve implicit
   zero initialization, array sizes and cryptographic constant bytes.
3. Add resource wrappers where ownership is clear, starting with WinAPI handles
   that have a single cleanup function. Cover errors and asynchronous callbacks
   before extending RAII to shared buffers or objects.
4. Test another compiler once the required extensions have been reduced. Treat
   Unix/macOS builds and Windows CI as separate tasks with pinned dependencies.
5. Define UI changes before implementation, including DPI scaling, accessibility
   and session handling. Test interactions manually as well as process startup.

Before each code change, run the existing test suite and record compiler
versions. Compare new warnings against the baseline. Changes to PPK files,
settings formats, SSH, cryptographic primitives, coroutines, secret-memory
clearing or callback ABI need tests beyond a successful build.
