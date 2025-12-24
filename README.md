# Toralize
A lightweight LD_PRELOAD library that intercepts connect() calls and transparently routes TCP connections through a SOCKS4 proxy.

## Source
This project was originally presented by Dr. Jonas Birch on his YouTube channel. The code demonstrates advanced concepts in:
- Linux system programming
- LD_PRELOAD hooking techniques
- SOCKS protocol implementation
- Network interception and redirection

## Overview
Toralize is a shared library that, when preloaded, intercepts the standard connect() system call and redirects connections through a SOCKS4 proxy without requiring any code modifications to the target application. This is particularly useful for forcing applications that don't natively support proxies to route their traffic through Tor or other SOCKS4-compatible proxies.

## Features
- Transparent Proxy Routing: Automatically redirects TCP connections through a SOCKS4 proxy
- No Application Modifications: Works via LD_PRELOAD without changing application code
- SOCKS4 Protocol Support: Implements the SOCKS4 protocol for proxy communication
- Simple Configuration: Easy setup via environment variables or source configuration
- Lightweight: Minimal performance overhead
- Educational Value: Excellent example of LD_PRELOAD hooking and network interception

## Project Structure
```text
toralize/
├── toralize.h      # Header file with SOCKS4 protocol definitions
├── toralize.c      # Main implementation of the connect() interceptor
└── Makefile        # Build configuration
```

## Building
To build the shared library:

```bash
make
```
This will generate toralize.so in the current directory.

To clean up:

```bash
make clean
```

## Usage
## Basic Usage
Preload the library when launching your application:

```bash
LD_PRELOAD=./toralize.so your_application
```

## Configuration
The proxy settings are configured in toralize.h:

```c
#define PROXY       "127.0.0.1"    // Proxy server address
#define PROXYPORT   9050           // Proxy server port (Tor default: 9050)
#define USERNAME    "toraliz"      // SOCKS4 username identifier
```

Modify these values to match your proxy server configuration.

## Example with Tor
To route an application's traffic through Tor (assuming Tor is running with default settings):

```bash
LD_PRELOAD=./toralize.so curl https://example.com
```

## Example with Custom Proxy
For a custom SOCKS4 proxy at 192.168.1.100:1080:

1. Edit toralize.h:

```c
#define PROXY       "192.168.1.100"
#define PROXYPORT   1080
```

2. Rebuild:

```bash
make clean && make
```

3. Run your application:

```bash
LD_PRELOAD=./toralize.so your_app
```

## How It Works
## Interception Mechanism
The library uses dlsym(RTLD_NEXT, "connect") to obtain a pointer to the original connect() function, then provides its own implementation that:

- Establishes a connection to the SOCKS4 proxy
- Sends a SOCKS4 CONNECT request with the original destination
- If successful, duplicates the proxy socket to the application's socket descriptor
- Returns control to the application with the proxied connection

## SOCKS4 Protocol Implementation
The library implements the SOCKS4 protocol with the following request format:

```text
+----+----+----+----+----+----+----+----+----+----+....+----+
| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
+----+----+----+----+----+----+----+----+----+----+....+----+
    1    1      2              4           variable       1
```

Where:

- VN = SOCKS version (4)
- CD = Command code (1 = CONNECT)
- DSTPORT = Destination port (network byte order)
- DSTIP = Destination IP (network byte order)
- USERID = Username identifier

## Connection Flow
1. Application calls connect() with original destination
2. Toralize intercepts the call and establishes connection to SOCKS4 proxy
3. SOCKS4 CONNECT request is sent with original destination details
4. Proxy responds with success/failure
5. On success, the proxy socket is duplicated to the application's socket descriptor
6. Application continues with proxied connection

## 📚 Educational Value
This project serves as an excellent learning resource for:

1. LD_PRELOAD Technique: Learn how to intercept and override shared library functions
2. SOCKS Protocol: Understand SOCKS4 protocol implementation
3. Network Programming: Study socket programming and proxy communication
4. System Hooking: Explore practical applications of function hooking
5. Tor Integration: See how applications can be forced through Tor network

## Limitations
- SOCKS4 Only: Does not support SOCKS4a or SOCKS5 protocols
- TCP Only: Only intercepts TCP connections (via connect())
- IPv4 Only: Only supports IPv4 addresses
- Single Proxy: All connections go through the same configured proxy
- No Authentication: Only supports SOCKS4 without authentication\
- DNS Leaks: DNS resolution occurs locally (SOCKS4 limitation)

## Security Considerations
1. Transparent Traffic Routing: Be aware that all TCP connections from the application will be routed through the proxy
2. DNS Leaks: SOCKS4 requires IP addresses, so DNS resolution occurs locally (not through the proxy)
3. Error Handling: The library provides basic error handling but may not cover all edge cases
4. Thread Safety: Use with caution in multi-threaded applications

## Troubleshooting
## Connection Failures
- Verify the proxy server is running and accessible
- Check that the proxy supports SOCKS4 protocol
- Ensure firewall rules allow connections to the proxy

##Library Loading Issues
- Use ldd ./toralize.so to check for missing dependencies
- Ensure you have permission to preload libraries
- Some applications may have restrictions on LD_PRELOAD

## Debugging
For debugging output, you can add printf statements to toralize.c and rebuild.

## License
This educational project is provided as-is for learning purposes. The original implementation was shared by Dr. Jonas Birch in his educational content.

## 🙏 Acknowledgments
- Dr. Jonas Birch for sharing this educational project on his YouTube channel
- Based on the LD_PRELOAD technique for intercepting system calls
- SOCKS4 protocol specification
- The Tor Project for providing the proxy service use case

## 🔗 Related Resources
- LD_PRELOAD documentation
- SOCKS4 Protocol Specification
- Tor Project
- Dr. Jonas Birch's YouTube Channel - Check for the original video explaining this implementation
