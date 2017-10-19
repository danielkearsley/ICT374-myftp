# ICT374-myftp
Murdoch University group project for ICT374. Implement an FTP client and server in C on linux.

Designed and implemented by:
Clem
Daniel


## MYFTP protocol spec

### Syntax - formats of packets

  RRQ, WRQ
    | 2B     | nB       | 0 | 0-1  | 0 |
    | opcode | filename | 0 | mode | 0 |
  CMD
    | 2B     | nB      | 0 |
    | opcode | command | 0 |
  DATA
    | 2B     | 0-512 |
    | opcode | data  |
  ACK
    | 2B     |
    | opcode |
  ERROR
    | 2B     | 1B         | nB        | 0 |
    | opcode | error code | error msg | 0 |


### Semantics - packet types, file mode supported, commands supported, error codes

  RRQ/WRQ
    - text
    Modes:
      0) ascii
      1) binary

  CMD
    - Client sends a cmd packet with one of the following commands.
    - Server processes request and if successful returns a data packet.
    -
    Commands:
      -PWD
        data packet with current client directory string.
      -LPWD
        data packet with current server directory string.
      -DIR
        data packet with current client directory listing.
      -LDIR
        data packet with current server directory listing.
      -CD  [dirname]
        nothing or ack?
      -LCD [dirname]
        nothing
      -QUIT

  DATA
    - text

  ACK
    - text

  ERR
    - text
    Error codes:
    0) Illegal operation.
    1) file not found.
    2) file already exists.

### Timing - sequence of exchange

  WRQ:
    ->WRQ
    ->DATA, =512B :. more to come
    ->DATA, <512B :. end

  RRQ:
    ->RRQ
    <-DATA, =512B :. more to come
    <-DATA, <512B :. end

  CMD:
    ->CMD
    <-DATA, =512B :. more to come
    <-DATA, <512B :. end

  Error:
    ->CMD
    <-ERR[0]
    or
    ->RRQ
    <-ERR[1]
    or
    ->WRQ
    <-ERR[2]

