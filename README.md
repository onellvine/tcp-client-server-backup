##### python2: no
##### python3: yes

### Usage

This is a multihtreaded server that spawns a new thread upon receiving a new request. The program
can run on a terminal in a system with python3 installed.

To run the program:-

I) using the default arguments, use...

    python3 main.py

II) using user defined arguments, use...

    python3 main.py -ip_address 127.0.0.1 -p 65365

    ###### replace [127.0.0.1] with a desired ip address and [65365] with a desired unused port number

Repeat either (I)  or (II) in a new terminal window and observe the results.

### Expected results
On running the program for a second time, using the same command line arguments, the response
from the server includes a 'Thread-3', which indicates the server created a new thread for the
new request. This also indicates that the program is working as intended. It is able to create
a new thread upon receiving a request, in the form of a message sent by the client object.
