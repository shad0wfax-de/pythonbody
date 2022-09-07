import sys
import os
sys.path.insert(0, "../")
sys.path.insert(0, "./")
os.chdir("../")


import numpy as np
import pandas as pd
from utils.grav_pot import grav_pot
#import timeit
import datetime as dt

N = 100000

if __name__ == "__main__":
    df = pd.DataFrame({
        "M": np.random.rand(N),
        "X1": np.random.rand(N),
        "X2": np.random.rand(N),
        "X3": np.random.rand(N),
        })
    EPOT = {}
    EPOT_c_funcs = [ "unthreaded", "omp", "ocl", "cuda"]
    for c_func in EPOT_c_funcs:
        try:
            s = dt.datetime.now()
            #eval(f"EPOT_{c_func} = None")
            EPOT[c_func] = eval(f"grav_pot(df, c_func='{c_func}')")
            print(f"{c_func.replace('unthreaded','unthrd')}:\t{(dt.datetime.now() - s).microseconds/1000000 + (dt.datetime.now() - s).seconds}")
            if EPOT_c_funcs.index(c_func) != 0:
              np.testing.assert_allclose(EPOT[EPOT_c_funcs[EPOT_c_funcs.index(c_func) - 1]], EPOT[c_func])
        except:
            continue