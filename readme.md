#CSC360 - Assignment 3

#Author: Adam Page

#Date: August 2nd, 2020

This assignment deals with Files systems, namely the Microsoft's FAT.
It assumes the working directories on the system running it are UNIX based.
This assignment consists of 4 programs:
- diskinfo
- disklist
- diskget
- diskput

diskinfo reads the supplied file system and display information about it.
For example:
  """
  Super block information: 
  Block size: 512
  Block count: 15360
  FAT starts: 1
  FAT blocks: 120
  Root directory start: 121
  Root directory blocks: 4
  FAT information: 
  Free Blocks: 15235
  Reserved Blocks: 121
  Allocated Blocks: 4
  """

disklist lists the contents of the root directory in the supplied file system
For example:
  """
  F 91 readme.txt 2016/03/02 12:20:19
  F 56 bar.txt 2016/03/02 12:20:43
  F 25600 file3.txt 2016/03/02 12:20:51
  F 51200 file4.txt 2016/03/02 12:20:55
  F 38512 ls_mac 2016/03/02 12:21:42
  """

diskget retieves the given file from the file system

diskput puts the given file onto the file system
