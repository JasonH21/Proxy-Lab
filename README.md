[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-c66648af7eb3fe8bc4f294546bfd86ef473780cde1dea487d3c4ff354943c9ae.svg)](https://classroom.github.com/online_ide?assignment_repo_id=9498892&assignment_repo_type=AssignmentRepo)
####################################################################
# CS:APP Proxy Lab
#
# Student Source Files
#
####################################################################

This directory contains the files you will need for the CS:APP Proxy
Lab.

proxy.c
csapp.h
csapp.c
    These are starter files.  csapp.c and csapp.h are described in
    your textbook.  The version here removes some of the wrapper
    functions, because they do not support the kind of error handling
    that a server program should.

    You may make any changes you like proxy.c.  And you may
    create and handin any additional files you like.

    Please do not modify csapp.h or csapp.c.  If you need different
    versions of the functions they provide, put them in a different
    location and use different names.

Makefile
    This is the makefile that builds the proxy program.  Type "make"
    to build your solution, or "make clean" followed by "make" for a
    fresh build. 

    Type "make all" to compile both your proxy and the Tiny web server.

    Type "make handin" to create the tarfile that you will be handing
    in.

    You should modify to include any additional code files you create.
    Autolab will use your Makefile to build your proxy from source.

port-for-user.pl
    Generates a random port for a particular user
    usage: ./port-for-user.pl <AndrewID>

driver.sh
    The autograder code used by Autolab
    usage: './driver.sh check' for the checkpoint,
    	   or './driver.sh' for the final submission

pxy
     PxyDrive testing framework

tests
     Test files used by Pxydrive

tiny
    Tiny Web server from the CS:APP text