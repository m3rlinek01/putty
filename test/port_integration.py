"""Exercise the compiled clients against a temporary loopback-only SSH/SFTP server.

Requires Paramiko. No external host, saved PuTTY session or persistent key is used.
"""
import base64
import hashlib
import json
import logging
import os
from pathlib import Path
import queue
import socket
import subprocess
import sys
import tempfile
import threading
import time
import uuid

import paramiko

logging.getLogger('paramiko').setLevel(logging.CRITICAL)
root = Path(__file__).resolve().parents[1]
bin_dir = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root / 'build'
host_key = paramiko.RSAKey.generate(2048)
fingerprint = 'SHA256:' + base64.b64encode(hashlib.sha256(host_key.asbytes()).digest()).decode().rstrip('=')
stop = threading.Event()
transports = []
auth_attempts = []
results = []
hold_ready = threading.Event()

class Server(paramiko.ServerInterface):
    def __init__(self):
        self.commands = queue.Queue()
    def get_allowed_auths(self, username):
        return 'password'
    def check_auth_password(self, username, password):
        auth_attempts.append(username)
        return paramiko.AUTH_SUCCESSFUL if (username, password) == ('tester', 'local-test-only') else paramiko.AUTH_FAILED
    def check_channel_request(self, kind, chanid):
        return paramiko.OPEN_SUCCEEDED if kind == 'session' else paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED
    def check_channel_exec_request(self, channel, command):
        if command == b'port-hold':
            hold_ready.set()
            return True
        if command != b'port-check':
            return False
        self.commands.put(channel)
        return True

class Handle(paramiko.SFTPHandle):
    def stat(self):
        return paramiko.SFTPAttributes.from_stat(os.fstat((self.readfile or self.writefile).fileno()))

class SFTP(paramiko.SFTPServerInterface):
    def __init__(self, server, *, directory):
        super().__init__(server)
        self.directory = Path(directory).resolve()
    def local(self, path):
        result = (self.directory / path.lstrip('/')).resolve()
        if not result.is_relative_to(self.directory):
            raise PermissionError('Path escapes temporary test directory')
        return result
    def canonicalize(self, path):
        self.local(path)
        return '/' + path.strip('/') if path.strip('/') not in ('', '.') else '/'
    def stat(self, path):
        try:
            return paramiko.SFTPAttributes.from_stat(self.local(path).stat())
        except OSError as error:
            return paramiko.SFTPServer.convert_errno(error.errno)
    lstat = stat
    def list_folder(self, path):
        entries = []
        for item in self.local(path).iterdir():
            attrs = paramiko.SFTPAttributes.from_stat(item.stat())
            attrs.filename = item.name
            entries.append(attrs)
        return entries
    def open(self, path, flags, attr):
        try:
            fd = os.open(self.local(path), flags | os.O_BINARY, 0o600)
            mode = 'wb' if flags & os.O_WRONLY else 'r+b' if flags & os.O_RDWR else 'rb'
            file = os.fdopen(fd, mode)
            handle = Handle(flags)
            handle.readfile = None if flags & os.O_WRONLY else file
            handle.writefile = file if flags & (os.O_WRONLY | os.O_RDWR) else None
            return handle
        except OSError as error:
            return paramiko.SFTPServer.convert_errno(error.errno)
    def mkdir(self, path, attr):
        try:
            self.local(path).mkdir()
            return paramiko.SFTP_OK
        except OSError as error:
            return paramiko.SFTPServer.convert_errno(error.errno)

def serve_client(client, directory):
    transport = paramiko.Transport(client)
    transports.append(transport)
    server = Server()
    try:
        transport.add_server_key(host_key)
        transport.set_subsystem_handler('sftp', paramiko.SFTPServer, SFTP, directory=directory)
        transport.start_server(server=server)
        channels = []
        while transport.is_active() and not stop.is_set():
            channel = transport.accept(0.05)
            if channel is not None:
                channels.append(channel)  # Retain channels while a subsystem is active.
            try:
                channel = server.commands.get_nowait()
            except queue.Empty:
                continue
            time.sleep(0.05)  # Allow the exec request acknowledgement to be delivered.
            channel.sendall(b'PUTTY_CPP_SSH_OK\n')
            channel.send_exit_status(0)
            channel.shutdown_write()
            channel.close()
    except (EOFError, OSError, paramiko.SSHException):
        pass  # A rejected host key deliberately closes a connection during handshake.
    finally:
        transport.close()

def accept_clients(listener, directory):
    while not stop.is_set():
        try:
            client, _ = listener.accept()
        except socket.timeout:
            continue
        except OSError:
            break
        threading.Thread(target=serve_client, args=(client, directory), daemon=True).start()

def run(name, executable, arguments, *, expected=0):
    proc = subprocess.run([str(bin_dir / executable)] + arguments, capture_output=True, text=True, timeout=30)
    record = {'test':name, 'exit':proc.returncode, 'expected_exit':expected,
              'stdout':proc.stdout, 'stderr':proc.stderr}
    results.append(record)
    assert proc.returncode == expected, record
    print(name, 'PASS', flush=True)
    return proc

with tempfile.TemporaryDirectory(prefix='putty-cpp-integration-') as work:
    work = Path(work)
    remote = work / 'server'
    remote.mkdir()
    listener = socket.socket()
    listener.bind(('127.0.0.1', 0))
    listener.listen(8)
    listener.settimeout(0.2)
    port = str(listener.getsockname()[1])
    thread = threading.Thread(target=accept_clients, args=(listener, remote), daemon=True)
    thread.start()
    common = ['-batch', '-noagent', '-P', port, '-hostkey', fingerprint, '-pw', 'local-test-only']
    try:
        proc = run('plink SSH command', 'plink.exe', common + ['-l', 'tester', '127.0.0.1', 'port-check'])
        assert proc.stdout.strip() == 'PUTTY_CPP_SSH_OK'
        bad = common.copy()
        bad[bad.index(fingerprint)] = 'SHA256:' + 'A'*43
        run('plink rejects wrong host key', 'plink.exe', bad + ['-l', 'tester', '127.0.0.1', 'port-check'], expected=1)
        payload = bytes(range(256)) * 4096
        source = work / 'input.bin'
        source.write_bytes(payload)
        run('pscp SFTP upload', 'pscp.exe', ['-sftp'] + common + [str(source), 'tester@127.0.0.1:payload.bin'])
        assert (remote / 'payload.bin').read_bytes() == payload
        downloaded = work / 'downloaded.bin'
        run('pscp SFTP download', 'pscp.exe', ['-sftp'] + common + ['tester@127.0.0.1:payload.bin', str(downloaded)])
        assert downloaded.read_bytes() == payload
        commands = work / 'batch.txt'
        commands.write_text(f'put "{source.as_posix()}" via-psftp.bin\nget via-psftp.bin "{(work / "psftp.bin").as_posix()}"\nquit\n')
        run('psftp batch upload/download', 'psftp.exe', common + ['-b', str(commands), 'tester@127.0.0.1'])
        assert (work / 'psftp.bin').read_bytes() == payload
        assert (remote / 'via-psftp.bin').read_bytes() == payload

        # A private transfer endpoint must identify exactly one authenticated
        # window, regardless of host/user placeholders passed by the GUI.
        token = uuid.uuid4().hex
        upstream = subprocess.Popen(
            [str(bin_dir / 'plink.exe')] + common +
            ['-share', '-share-id', token, '-l', 'tester', '127.0.0.1', 'port-hold'],
            stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            assert hold_ready.wait(15), 'Private sharing upstream did not authenticate'
            existing_connections = len(transports)
            existing_authentications = len(auth_attempts)
            # Open additional terminal channels with no password or key. The
            # same endpoint must preserve the original authenticated identity.
            for label in ('duplicate authenticated session', 'duplicate of duplicate'):
                duplicate = run(label, 'plink.exe',
                    ['-batch', '-noagent', '-share', '-share-id', token,
                     '-l', 'not-the-original-user', 'invalid.invalid', 'port-check'])
                assert duplicate.stdout.strip() == 'PUTTY_CPP_SSH_OK'
                assert len(transports) == existing_connections, 'Duplicate opened another TCP connection'
                assert len(auth_attempts) == existing_authentications, 'Duplicate authenticated again'
            private = ['-batch', '-share-id', token]
            # The remote path, local path and recursively enumerated filename
            # all contain characters unavailable in a typical Windows ACP.
            tree = work / 'drag folder \u6f22\U0001f642'
            nested = tree / 'nested directory'
            nested.mkdir(parents=True)
            (nested / 'za\u017c\u00f3\u0142\u0107 \u6f22.txt').write_bytes(payload)
            progress = run('private recursive Unicode upload', 'pscp.exe', private +
                ['-r', '--', str(tree), 'unused@invalid.invalid:/'])
            records = [line.split('\t') for line in progress.stdout.splitlines()
                       if line.startswith('PUTTY-PROGRESS\t')]
            assert records, 'Missing structured GUI progress records'
            for record in records:
                assert len(record) == 6, record
                size, done, speed, eta = map(int, record[1:5])
                assert 0 <= done <= size and speed >= 0 and eta >= 0, record
                assert bytes.fromhex(record[5]).decode('utf-8') == 'za\u017c\u00f3\u0142\u0107 \u6f22.txt'
            assert any(int(r[2]) == len(payload) for r in records), records
            assert (remote / tree.name / nested.name / 'za\u017c\u00f3\u0142\u0107 \u6f22.txt').read_bytes() == payload
            target = work / 'download \u6f22\U0001f642'
            target.mkdir()
            run('private recursive Unicode download', 'pscp.exe', private +
                ['-r', '--', f'unused@invalid.invalid:/{tree.name}', str(target)])
            assert (target / tree.name / nested.name / 'za\u017c\u00f3\u0142\u0107 \u6f22.txt').read_bytes() == payload
            # Windows cannot represent '*' or '?' in a local filename, but
            # brackets are valid and must be escaped for PSCP's remote parser.
            (remote / 'literal[1].zip').write_bytes(payload)
            literal = work / 'literal-result.zip'
            run('private literal remote wildcard quoting', 'pscp.exe', private +
                [r'unused@invalid.invalid:/literal\[1\].zip', str(literal)])
            assert literal.read_bytes() == payload
            junction = work / 'junction-to-dropped-tree'
            subprocess.run([os.environ.get('COMSPEC', 'cmd.exe'), '/d', '/c',
                            'mklink', '/J', str(junction), str(tree)],
                           check=True, capture_output=True, timeout=10)
            rejected = run('private upload refuses directory junction', 'pscp.exe',
                           private + ['-r', '--', str(junction),
                                      'unused@invalid.invalid:/'], expected=1)
            assert 'junction' in rejected.stderr
            assert not (remote / junction.name).exists()
            assert len(transports) == existing_connections, 'Transfer opened a new TCP connection'
            run('private wrong window fails without network fallback', 'pscp.exe',
                ['-batch', '-share-id', uuid.uuid4().hex, '-P', port,
                 str(source), 'tester@127.0.0.1:wrong-window.bin'], expected=1)
            time.sleep(0.1)
            assert len(transports) == existing_connections, 'Missing endpoint fell back to network'
            assert not (remote / 'wrong-window.bin').exists()
        finally:
            upstream.kill()
            upstream.communicate(timeout=10)
        run('private closed session fails without network fallback', 'pscp.exe',
            ['-batch', '-share-id', token, '-P', port,
             str(source), 'tester@127.0.0.1:closed-window.bin'], expected=1)
        time.sleep(0.1)
        assert len(transports) == existing_connections, 'Closed endpoint fell back to network'
        assert not (remote / 'closed-window.bin').exists()
    finally:
        stop.set()
        listener.close()
        for transport in transports:
            transport.close()
        thread.join(timeout=2)

report = {'verdict':'PASS', 'listener':'127.0.0.1, temporary port',
          'payload_bytes':len(payload), 'payload_sha256':hashlib.sha256(payload).hexdigest(), 'checks':results}
(root / 'validation').mkdir(exist_ok=True)
(root / 'validation/integration.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print('All SSH/SFTP integration checks passed.')
