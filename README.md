LEEK
====

### **L**ittle **E**ducational **E**lectronic **C**omputer

This project has been move to GitLab, you can find the repo [here](https://gitlab.com/hamsterjam/leek).

This is a project to design and implement a toy 16-but computation system from the hardware up.
The aim is to implement a full computation platform (first as a virtual machine, maybe actual hardware eventually)
and build up software for it to the point where I have a working system.

The project is split into modules each doing a specific task related to building a computation platform

Virtual Machine
---------------

The virtual machine is a program that emulates the (theoretical) hardware of a LEEK computer.
It runs LEEK byte code generated by one of the other tools.
Although it has fairly limitied capabilities at the moment, it can be upgraded with additional peripheral i/o devices to make it actually useful.
As of now, it is only really useful for very simple test programs to verify that the instruction set is expressive and that the operations are being performed correctly.

Assembler
---------

A very simple assembler so I can sort of write some actual programs.
It occurs to me right now writting this that I haven't actually documented it.

Sleek
-----

A compiled language as a replacement for C and C++ for LEEK projects.
It is an object oriented language that lies somewhere between C and C++ in terms of complexity
My motivation behind this is not only to not have to rely on C++ for all my projects,
but also to fix a lot of the problems I find in using C and C++.

This will probably become it's own project later that compiles to x86 because it's way more fun to use than C++.
