<<<<<<< HEAD
NAME := kernel
SRCS := kernel.c
export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
export ARCH := x86_64-qemu
=======
NAME           := kernel
SRCS           := framework/main.c $(shell find -L ./src/ -name "*.c")
INC_PATH       := include/ framework/

export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
export ARCH    := x86_64-qemu
>>>>>>> b1bcf324b7fb436bdb8bd34cffe6d9a13f12c62c
endif

include $(AM_HOME)/Makefile
include ../oslabs.mk
image: git
