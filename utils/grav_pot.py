from ctypes import cdll, c_double, c_int
import pathlib
import pandas as pd
import numpy as np

def grav_pot(data: pd.DataFrame,
             G: float = 1,
             num_threads: int = None,
             c_func = "cuda"
             ):
    if c_func not in ["cuda","ocl","omp","unthreaded", None]:
        raise ValueError("c_func must be either cuda, ocl, omp, unthreaded or None")

    if num_threads is None:
        from multiprocessing import cpu_count
        num_threads = cpu_count()
    N = data.shape[0]

    EPOT = (c_double * N)(*np.zeros(N))
    lib = None
    func = None
    if c_func == "cuda" and pathlib.Path("pythonbody/ffi/.libs/libgrav_pot_cuda.so").is_file():
        lib = cdll.LoadLibrary("pythonbody/ffi/.libs/libgrav_pot_cuda.so")  
        func = eval(f"lib.grav_pot")
    else:
        lib = cdll.LoadLibrary("pythonbody/ffi/.libs/libgrav_pot.so")
        if c_func is not None:
            func = eval(f"lib.grav_pot_{c_func}")
        else:
            func = eval(f"lib.grav_pot")
    
    func.argtypes = [
            c_double * N, # M
            c_double * N, # X1 
            c_double * N, # X2
            c_double * N, # X3
            c_double * N, # EPOT (results)
            c_int,        # N
            ]#c_int,        # num_threads
            #]
    func.restype = c_int 

    func((c_double * N)(*data["M"].values),
                (c_double * N)(*data["X1"].values),
                (c_double * N)(*data["X2"].values),
                (c_double * N)(*data["X3"].values),
                EPOT,
                N,
                num_threads
                )
    return np.array(EPOT)

