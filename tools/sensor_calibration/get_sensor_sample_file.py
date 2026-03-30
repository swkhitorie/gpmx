#! /usr/bin/env python3

try:
	import numpy as np
except:
	print("[python] please install numpy!!!")

def readsamplefile(path, x_idx, y_idx, z_idx):
    hfile = open(path, "r")
    x_lst = []
    y_lst = []
    z_lst = []
    cnt = 0
    while True:
        line = hfile.readline()
        if not line:
            break
        words = line.split(", ")
        x_lst.append(float(words[x_idx].strip()))
        y_lst.append(float(words[y_idx].strip()))
        z_lst.append(float(words[z_idx].strip()))
        cnt = cnt + 1
    hfile.close()
    return x_lst, y_lst, z_lst, cnt