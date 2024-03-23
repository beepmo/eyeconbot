# -*- coding: utf-8 -*-
"""
Created on Sat Mar 23 09:11:57 2024

@author: beepmo.stu
"""

byte = b'\xff\x03'
print(byte)
print(type(byte))
print(len(byte))

print(int.from_bytes(byte,"little",signed=False))
print(int.from_bytes(byte,"big",signed=False))