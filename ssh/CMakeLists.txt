add_library(sshcommon OBJECT
  bpp1.cpp
  bpp2.cpp
  bpp-bare.cpp
  ca-config.cpp
  censor1.cpp
  censor2.cpp
  common.cpp
  connection1.cpp
  connection2.cpp
  crc-attack-detector.cpp
  gssc.cpp
  login1.cpp
  nocompress.cpp
  pgssapi.cpp
  portfwd.cpp
  ../sshpubk.cpp
  ../sshrand.cpp
  transient-hostkey-cache.cpp
  transport2.cpp
  verstring.cpp
  x11fwd.cpp
  zlib.cpp)

add_library(sftpcommon OBJECT sftpcommon.cpp)

add_library(sshclient STATIC
  agentf.cpp
  connection1-client.cpp
  connection2-client.cpp
  kex2-client.cpp
  mainchan.cpp
  sharing.cpp
  ssh.cpp
  userauth2-client.cpp
  $<TARGET_OBJECTS:sshcommon>
  $<TARGET_OBJECTS:all-backends>
  $<TARGET_OBJECTS:logging>)

add_library(sshserver STATIC
  connection1-server.cpp
  connection2-server.cpp
  kex2-server.cpp
  login1-server.cpp
  server.cpp
  sesschan.cpp
  sftpserver.cpp
  userauth2-server.cpp
  $<TARGET_OBJECTS:sftpcommon>
  $<TARGET_OBJECTS:sshcommon>)

add_sources_from_current_dir(sftpclient sftp.cpp)
target_sources(sftpclient PRIVATE $<TARGET_OBJECTS:sftpcommon>)
