# Test output

Run `build-cpp.ps1 -Test` for x64 or add `-Architecture x86` for the 32-bit build.
CTest writes `build/Testing/Temporary/LastTest.log` or
`build-x86/Testing/Temporary/LastTest.log`. Local SSH/SFTP tests write
`validation/integration.json`.

Generated reports contain local environment paths and are ignored by Git.
See [transfer validation](../docs/FILE-TRANSFER-VALIDATION.md) for coverage.
