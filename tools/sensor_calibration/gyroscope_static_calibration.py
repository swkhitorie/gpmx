#! /usr/bin/env python3

print("\
\ngyro calibration model is:\n\
┌   ┐   ┌   ┐   ┌      ┐ \n\
│gx'│   │gx │   │offstx│ \n\
│gy'│ = │gy │ - │offsty│ \n\
│gz'│   │gz │   │offstz│ \n\
└   ┘   └   ┘   └      ┘ \n\
")

import get_sensor_sample_file
try:
	import numpy as np
except:
	print("[python] please install numpy!!!")

file_path = input("\nput gyro sample file in (format:x, y, z)\n")
result = get_sensor_sample_file.readsamplefile(file_path, 1, 2, 3)

x_lst = result[0]
y_lst = result[1]
z_lst = result[2]
cnt_al = result[3]

x_a = np.array(x_lst)
y_a = np.array(y_lst)
z_a = np.array(z_lst)

print("sample num: %d\n" % result[3])

print("\
offset vector:\n\
┌      ┐   ┌ \t \t┐\n\
│offstx│   │%lf \t│\n\
│offsty│ = │%lf \t│\n\
│offstz│   │%lf \t│\n\
└      ┘   └ \t \t┘\n\
" % (x_a.mean(), y_a.mean(), z_a.mean()))


