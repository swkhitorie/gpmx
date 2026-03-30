#! /usr/bin/env python3

GRAVITY_MSS = 9.7843
ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID = 0
ACCEL_CAL_ELLIPSOID = 1
ACCEL_CAL_MAX_NUM_PARAMS = 9
ACCEL_CAL_TOLERANCE = 0.1
MAX_ITERATIONS = 50
M_PI_F = 3.14159265358

print("\
\naccel calibration model is:\n\
┌   ┐   ┌                                   ┐  ┌   ┐   ┌      ┐ \n\
│ax'│   │diag(0)    offdiag(0)    offdiag(1)│  │ax │   │offstx│ \n\
│ay'│ = │offdiag(0)    diag(1)    offdiag(2)│ (│ay │ + │offsty│)\n\
│az'│   │offdiag(1)    offdiag(2)    diag(2)│  │az │   │offstz│ \n\
└   ┘   └                                   ┘  └   ┘   └      ┘ \n\
")

import get_sensor_sample_file
import math
try:
	import numpy as np
except:
	print("[python] please install numpy!!!")

_conf_fit_type = ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID
_conf_num_samples = 6
_conf_tolerance = 0.1

faces = 2 * _conf_num_samples - 4;
a = (4.0 * M_PI_F / (3.0 * faces)) + M_PI_F / 3.0;
theta = 0.5 * math.acos(math.cos(a) / (1.0 - math.cos(a)));
_min_sample_dist = GRAVITY_MSS * 2*math.sin(theta/2);

accel_sample = np.zeros((_conf_num_samples, 3))

#########################################################
#              ACCEL SAMPLE FROM FILE                   #
#########################################################

for i in range(0, _conf_num_samples):
	file_path = input("\nput accel sample file(format:x, y, z) -- (%d/%d), unit in m/s^2 \n" % (i+1, _conf_num_samples))
	result = get_sensor_sample_file.readsamplefile(file_path, 1, 2, 3)
	accel_sample[i,0] = np.array(result[0]).mean()
	accel_sample[i,1] = np.array(result[1]).mean()
	accel_sample[i,2] = np.array(result[2]).mean()
	print("\nmean: %lf %lf %lf\r\n" % (accel_sample[i,0], accel_sample[i,1], accel_sample[i,2]))

#########################################################
#              Gauss Newton fit model2                  #
#########################################################

param_offset = np.array([0.0, 0.0, 0.0])
param_diag = np.array([1.0, 1.0, 1.0])
param_offdiag = np.array([0.0, 0.0, 0.0])

def get_num_params():
	if (_conf_fit_type == ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID):
		return 6
	if (_conf_fit_type == ACCEL_CAL_ELLIPSOID):
		return 9

def calc_residual(sample, offset, diag, offdiag):
	M = np.array(\
	[[diag[0], offdiag[0], offdiag[1]]\
	,[offdiag[0], diag[1], offdiag[2]],\
	[offdiag[1], offdiag[2], diag[2]]])
	return GRAVITY_MSS - np.linalg.norm(((M * sample + offset)))

def calc_mean_squared_residuals(sp, offset, diag, offdiag):
	sum = 0.0
	for i in range(0, _conf_num_samples):
		sample = sp[i,:]
		resid = calc_residual(sample, offset, diag, offdiag)
		sum += np.square(resid)
	sum /= _conf_num_samples
	return sum

def calc_jacob(sample, offset, diag, offdiag):
	ret = np.zeros(ACCEL_CAL_MAX_NUM_PARAMS)
	M = np.array(\
	[[diag[0], offdiag[0], offdiag[1]]\
	,[offdiag[0], diag[1], offdiag[2]],\
	[offdiag[1], offdiag[2], diag[2]]])
	A = (diag[0]*(sample[0]+offset[0])) + (offdiag[0]*(sample[1]+offset[1])) + (offdiag[1]*(sample[2]+offset[2]))
	B = (offdiag[0]*(sample[0]+offset[0])) + (diag[1]*(sample[1]+offset[1])) + (offdiag[2]*(sample[2]+offset[2]))
	C = (offdiag[1]*(sample[0]+offset[0])) + (offdiag[2]*(sample[1]+offset[1])) + (diag[2]*(sample[2]+offset[2]))
	tmp = M * (sample + offset)
	length = np.linalg.norm(tmp)
	ret[0] = -1.0*(((diag[0]*A) + (offdiag[0]*B) + (offdiag[1]*C))/length)
	ret[1] = -1.0*(((offdiag[0]*A) + (diag[1]*B) + (offdiag[2]*C))/length)
	ret[2] = -1.0*(((offdiag[1]*A) + (offdiag[2]*B) + (diag[2]*C))/length)

	ret[3] = -1.0*((sample[0] + offset[0])*A)/length
	ret[4] = -1.0*((sample[1] + offset[1])*B)/length
	ret[5] = -1.0*((sample[2] + offset[2])*C)/length

	ret[6] = -1.0*(((sample[1] + offset[1])*A) + ((sample[0] + offset[0])*B))/length
	ret[7] = -1.0*(((sample[2] + offset[2])*A) + ((sample[0] + offset[0])*C))/length
	ret[8] = -1.0*(((sample[2] + offset[2])*B) + ((sample[1] + offset[1])*C))/length
	return ret

fitness = calc_mean_squared_residuals(accel_sample, param_offset, param_diag, param_offdiag)
min_fitness = fitness
num_iterations = 0
fit_offset = np.array([0.0, 0.0, 0.0])
fit_diag = np.array([1.0, 1.0, 1.0])
fit_offdiag = np.array([0.0, 0.0, 0.0])

while (num_iterations < 50):
	JTJ = np.zeros(ACCEL_CAL_MAX_NUM_PARAMS*ACCEL_CAL_MAX_NUM_PARAMS)
	JTFI = np.zeros(ACCEL_CAL_MAX_NUM_PARAMS)
	
	for i in range(0, _conf_num_samples):
		samp = accel_sample[i,:]
		jacob = np.zeros(ACCEL_CAL_MAX_NUM_PARAMS)
		jacob = calc_jacob(samp, fit_offset, fit_diag, fit_offdiag)
		for j in range(0, get_num_params()):
			for k in range(0, get_num_params()):
				JTJ[j*get_num_params()+k] += jacob[j] * jacob[k];
			JTFI[j] += jacob[j] * calc_residual(samp, fit_offset, fit_diag, fit_offdiag);

	JTJ_t = np.zeros((get_num_params(), get_num_params()))
	JTJ_inv_t = np.zeros((get_num_params(), get_num_params()))
	for i in range(0, get_num_params()):
		for j in range(0, get_num_params()):
			JTJ_t[i, j] = JTJ[i * get_num_params() + j]

	JTJ_inv_t = np.linalg.pinv(JTJ_t)
	for i in range(0, get_num_params()):
		for j in range(0, get_num_params()):
			if (i < 3):
				fit_offset[i] -= JTFI[j] * JTJ_inv_t[i, j]
			if (i > 3 and i < 6):
				fit_diag[i - 3] -= JTFI[j] * JTJ_inv_t[i, j]				
			if (i > 6 and i < 9):
				fit_offdiag[i - 6] -= JTFI[j] * JTJ_inv_t[i, j]

	fitness = calc_mean_squared_residuals(accel_sample, fit_offset, fit_diag, fit_offdiag)
	if (fitness < min_fitness):
		min_fitness = fitness
		param_offset = fit_offset
		param_diag = fit_diag
		param_offdiag = fit_offdiag
		
	num_iterations = num_iterations + 1

if (fitness < _conf_tolerance):
	print("fitness ok %f" % fitness)
else:
	print("fitness bad %f" % fitness)

if (math.fabs(param_offset[0]) > 3.5 or\
	math.fabs(param_offset[1]) > 3.5 or\
	math.fabs(param_offset[2]) > 3.5 or\
	math.fabs(param_diag[0] - 1.0) > 0.2 or\
	math.fabs(param_diag[1] - 1.0) > 0.2 or\
	math.fabs(param_diag[2] - 1.0) > 0.2):
	print("\ncalibration over, the result is too bad, calib failed\n")
else:
	print("\ncalibration over, the result is normal\n")

print("\
\naccel diag params:\n\
┌  \t\t \t\t\t    ┐\n\
│%lf \t %lf \t  %lf│\n\
│%lf \t %lf \t  %lf│\n\
│%lf \t %lf \t  %lf│\n\
└  \t\t \t\t\t    ┘\n\
" % (param_diag[0], param_offdiag[0], param_offdiag[1],\
param_offdiag[0], param_diag[1], param_offdiag[2],\
param_offdiag[1], param_offdiag[2], param_diag[2]))

print("\
naccel offset:\n\
┌      ┐   ┌ \t \t┐\n\
│offstx│   │%lf \t│\n\
│offsty│ = │%lf \t│\n\
│offstz│   │%lf \t│\n\
└      ┘   └ \t \t┘\n\
" % (param_offset[0], param_offset[1], param_offset[2]))

accel_sample_tst = np.zeros(3)
accel_sample_tst[0] = param_diag[0] * (accel_sample[0,0] + param_offset[0])
accel_sample_tst[1] = param_diag[1] * (accel_sample[0,1] + param_offset[1])
accel_sample_tst[2] = param_diag[2] * (accel_sample[0,2] + param_offset[2])
print("\nbefore value: %lf %lf %lf\r\n" % (accel_sample[0,0], accel_sample[0,1], accel_sample[0,2]))
print("\nafter value: %lf %lf %lf\r\n" % (accel_sample_tst[0], accel_sample_tst[1], accel_sample_tst[2]))
