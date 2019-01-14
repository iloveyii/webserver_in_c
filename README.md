Webserver in C
===============================

This a light webserver developed in C programming language using techniques like fork, pre-fork. It can serve static html files
and images.


![Screenshot](http://webserverc.softhem.se/screenshot.png)

# [Demo](http://webserverc.softhem.se:8888/)

INSTALLATIONS
---------------
  * Clone the repository `git clone git@github.com:iloveyii/webserver_in_c.git`.
  * Compile the source code using cc `cc lab2.c -o webserver`.
  * Then run executable `./webserver`.
  * You can adjust settings like mime types and docroot in file `mime.types and lab3-config` file as per your environment.

DIRECTORY STRUCTURE
-------------------

```
include                  contains header files and helper functions

www                      this is docroot
 
.lab3-config             this is the configuration file, settings for docroot and port number 

lab2.c                   this is the main.c file   
```