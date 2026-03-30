#! /usr/bin/env python3

GRAVITY_MSS = 9.7913
print("\
\naccel calibration method: 6 axis \
\naccel calibration model is:\n\
┌   ┐   ┌                ┐┌   ┐   ┌      ┐ \n\
│ax'│   │scalex    0    0││ax │   │offstx│ \n\
│ay'│ = │0    scaley    0││ay │ - │offsty│ \n\
│az'│   │0    0    scalez││az │   │offstz│ \n\
└   ┘   └                ┘└   ┘   └      ┘ \n\
")

import get_sensor_sample_file
import math
try:
	import numpy as np
except:
	print("[python] please install numpy!!!")

num_samples_collect = 6

accel_sample = np.zeros((num_samples_collect, 3))

#########################################################
#              ACCEL SAMPLE FROM FILE                   #
#########################################################

for i in range(0, num_samples_collect):
	file_path = input("\nput accel sample file(format:x, y, z) -- (%d/%d), unit in m/s^2 \n" % (i+1, num_samples_collect))
	result = get_sensor_sample_file.readsamplefile(file_path, 1, 2, 3)
	accel_sample[i,0] = np.array(result[0]).mean()
	accel_sample[i,1] = np.array(result[1]).mean()
	accel_sample[i,2] = np.array(result[2]).mean()
	print("\nmean: %lf %lf %lf\r\n" % (accel_sample[i,0], accel_sample[i,1], accel_sample[i,2]))

#########################################################
#              Gauss Newton fit                         #
#########################################################

accel_scale = np.zeros(3)
accel_offset = np.zeros(3)

def reset_gaussnewton(dS, JS):
	for j in range(0, 6):
		dS[j] = 0.0
		for k in range(0, 6):
			JS[j,k] = 0.0
	return

def fine_delta_gaussnewton(dS, JS, delta):
	mu = 0.0
	for i in range(0, 6):
		for j in range(i+1, 6):
			mu = JS[i,j] / JS[i,i]
			if (mu != 0.0):
				dS[j] -= mu * dS[i]
				for k in range(j, 6):
					JS[k, j] -= mu * JS[k, i]

	for i in range(5, -1, -1):
		dS[i] /= JS[i, i]
		JS[i, i] = 1.0
		for j in range(0, i):
			mu = JS[i, j]
			dS[j] -= mu * dS[i]
			JS[i, j] = 0.0
	for i in range(0, 6):
		delta[i] = dS[i]
	return

def update_gaussnewton(dS, JS, beta, data):
	b = 0.0
	dx = 0.0
	residual = 1.0
	jacobian = np.zeros(6)
	for j in range(0, 3):
		b = beta[3+j]
		dx = float(data[j]) - beta[j]
		residual -= b*b*dx*dx
		jacobian[j] = 2.0*b*b*dx
		jacobian[3+j] = -2.0*b*dx*dx
	for j in range(0, 6):
		dS[j] += jacobian[j] * residual
		for k in range(0, 6):
			JS[j, k] += jacobian[j] * jacobian[k]
	return



# params list init state
# [offx, offy, offz, scalex, scaley, scalez]
i = 0
num_iterations = 0
eps = 0.000000001
change = 100.0
data = np.zeros(6)
beta = np.zeros(6)
delta = np.zeros(6)
ds = np.zeros(6)
JS = np.zeros((6,6))

beta[0] = beta[1] = beta[2] = 0
beta[3] = beta[4] = beta[5] = 1.0 / GRAVITY_MSS

print("======================== gauss newton fitting start ========================\n")
print("start iterations, %lf %lf %lf %lf %lf %lf\n"\
        % (beta[0]*beta[3]*GRAVITY_MSS\
        , beta[1]*beta[4]*GRAVITY_MSS\
        , beta[2]*beta[5]*GRAVITY_MSS,\
        beta[3]*GRAVITY_MSS,\
        beta[4]*GRAVITY_MSS,\
        beta[5]*GRAVITY_MSS))
while (num_iterations < 20 and change > eps):
	num_iterations = num_iterations + 1

	reset_gaussnewton(ds, JS)

	for i in range(0,6):
		data[0] = accel_sample[i,0]
		data[1] = accel_sample[i,1]
		data[2] = accel_sample[i,2]
		update_gaussnewton(ds, JS, beta, data)

	fine_delta_gaussnewton(ds, JS, delta)

	change = delta[0] * delta[0] +\
			 delta[0] * delta[0] +\
			 delta[1] * delta[1] +\
			 delta[2] * delta[2] +\
			 delta[3] * delta[3] / (beta[3] * beta[3]) +\
			 delta[4] * delta[4] / (beta[4] * beta[4]) +\
			 delta[5] * delta[5] / (beta[5] * beta[5])

	for i in range(0, 6):
		beta[i] -= delta[i]
	print("%d times iterations, %lf %lf %lf %lf %lf %lf\n"\
        % (num_iterations\
        , beta[0]*beta[3]*GRAVITY_MSS\
        , beta[1]*beta[4]*GRAVITY_MSS\
        , beta[2]*beta[5]*GRAVITY_MSS,\
        beta[3]*GRAVITY_MSS,\
        beta[4]*GRAVITY_MSS,\
        beta[5]*GRAVITY_MSS))
print("======================== end fitting number of iteration")

accel_scale[0] = beta[3] * GRAVITY_MSS
accel_scale[1] = beta[4] * GRAVITY_MSS
accel_scale[2] = beta[5] * GRAVITY_MSS

accel_offset[0] = beta[0] * accel_scale[0]
accel_offset[1] = beta[1] * accel_scale[1]
accel_offset[2] = beta[2] * accel_scale[2]

print("\
\naccel scale params:\n\
┌  \t\t \t\t\t   ┐\n\
│%lf \t\t 0 \t\t  0│\n\
│0 \t\t %lf \t\t  0│\n\
│0 \t\t 0 \t\t%lf   │\n\
└  \t\t \t\t\t   ┘\n\
" % (accel_scale[0], accel_scale[1], accel_scale[2]))

print("\
naccel offset:\n\
┌      ┐   ┌ \t \t┐\n\
│offstx│   │%lf \t│\n\
│offsty│ = │%lf \t│\n\
│offstz│   │%lf \t│\n\
└      ┘   └ \t \t┘\n\
" % (accel_offset[0], accel_offset[1], accel_offset[2]))

if (math.fabs(accel_offset[0]) > 3.5 or\
	math.fabs(accel_offset[1]) > 3.5 or\
	math.fabs(accel_offset[2]) > 3.5 or\
	math.fabs(accel_scale[0] - 1.0) > 0.15 or\
	math.fabs(accel_scale[1] - 1.0) > 0.15 or\
	math.fabs(accel_scale[2] - 1.0) > 0.15):
	print("\ncalibration over, the result is too bad, calib failed\n")
else:
	print("\ncalibration over, the result is normal\n")


accel_sample_tst = np.zeros(3)
accel_sample_tst[0] = accel_scale[0] * accel_sample[0,0] - accel_offset[0]
accel_sample_tst[1] = accel_scale[1] * accel_sample[0,1] - accel_offset[1]
accel_sample_tst[2] = accel_scale[2] * accel_sample[0,2] - accel_offset[2]
print("\nbefore value: %lf %lf %lf\r\n" % (accel_sample[0,0], accel_sample[0,1], accel_sample[0,2]))
print("\nafter value: %lf %lf %lf\r\n" % (accel_sample_tst[0], accel_sample_tst[1], accel_sample_tst[2]))