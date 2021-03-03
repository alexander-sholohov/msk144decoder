#!/usr/bin/env python

CSDR_CMD = './csdr'
MSK144_CMD = './msk144decoder'

rtlsdr_sample_rate = 2048000
rtlsdr_center = 145000000
interest_frequency = 144360100 # MSK144 well known frequency + small correction of our rtlsdr's txco
shift = float(rtlsdr_center - interest_frequency) / rtlsdr_sample_rate
stage1_sample_rate = 32000
stage1_decimation = 64
stage1_bandpass_fir_low_cut = 0.0  # related to stage1_sample_rate
stage1_bandpass_fir_high_cut = 0.12  # related to stage1_sample_rate
stage1_bandpass_fir_transition = 0.06  # related to stage1_sample_rate
assert rtlsdr_sample_rate / stage1_decimation == stage1_sample_rate
stage2_resample = (3, 8)  
stage2_sample_rate = 12000  # do not change because msk144decoder accepts audio stream at this rate only
assert stage1_sample_rate * stage2_resample[0] / stage2_resample[1] == stage2_sample_rate


CMD_CHAIN = []
CMD_CHAIN.append( ['nc', '192.168.1.200', '2223'] )
# CMD_CHAIN.append( ['cat', '/dev/urandom'] )
CMD_CHAIN.append( [CSDR_CMD, 'convert_u8_f']  )
CMD_CHAIN.append( [CSDR_CMD, 'shift_addition_cc', shift] )
CMD_CHAIN.append( [CSDR_CMD, 'fir_decimate_cc', stage1_decimation, '0.005', 'HAMMING'] )
CMD_CHAIN.append( [CSDR_CMD, 'bandpass_fir_fft_cc', stage1_bandpass_fir_low_cut, stage1_bandpass_fir_high_cut, stage1_bandpass_fir_transition] )
CMD_CHAIN.append( [CSDR_CMD, 'realpart_cf'] )
CMD_CHAIN.append( [CSDR_CMD, 'rational_resampler_ff', stage2_resample[0], stage2_resample[1] ] ) 
CMD_CHAIN.append( [CSDR_CMD, 'gain_ff', '100.0'] ) 
CMD_CHAIN.append( [CSDR_CMD, 'limit_ff'] )
CMD_CHAIN.append( [CSDR_CMD, 'convert_f_s16'] )
CMD_CHAIN.append( [MSK144_CMD] )


print("echo \"msk144 f={}\"".format(interest_frequency))
s = " | ".join([" ".join([str(x) for x in elm]) for elm in CMD_CHAIN])
print(s)
