Web Server
==========
Summary
-------
A simple web server using BSD sockets.
Supported filetypes:
* JPEG
* GIF
* HTML

High Level Description
----------------------
1. Create Socket
2. Bind to port and IP Address
3. Listen for connections
4. Accept connections (3 way handshake)
5. Parse the message for the URL
6. See if the file exists on server
7. If it does add the appropriate content message (image or text)
8. Put the file into a buffer and send

Compiling
---------
The program is composed of only one source file so it’s quite easy to compile. You can choose to invoke the gcc command:
$ gcc serverfork.c -o serverfork
Or just use the included Makefile:
$ make
gcc serverfork.c -o serverfork
The Makefile comes with the added benefit of having a make clean option which will automatically delete the application and any object files.
$ make clean
To run the program simply type ./serverfork and the port number:
./serverfork 12345
