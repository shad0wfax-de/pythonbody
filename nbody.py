import logging
import re
import glob
from tqdm import tqdm
import pandas as pd
import numpy as np
import sys

from pythonbody.snap import snap
from pythonbody.cluster import cluster
from pythonbody import data_files


SUPPORTED_DATA_FILES = [i for i in dir(data_files) if ((i[:2] != "__") and (i != "pythonbody") and (i != "data_file"))]
SUPPORTED_COLS = {i: eval(f"data_files.{i}").COLS for i in dir(data_files) if ((i[:2] != "__") and (i != "pythonbody") and (i != "data_file"))}
CALCABLE_DATA = {key: [i for i in eval(f"dir(data_files.{key})") if "calc_" in i] for key in SUPPORTED_DATA_FILES}

class UnknownDataLoad(Exception):
    pass

class nbody:
    """
    Class for handling nbody results
    
    Attributes:
        version (str): nbody version used for the simulation
        data_path (str): Path where output files of nbody can be found
        job_id (list): Not required, but helps finding the correct (main out) file(s)
        
        _data (dict): contains data from nbody simulation
        _files (dict): contains files related to the simulation
    
    """
    def __init__(self, data_path: str = None, nb_stdout_files: list = None):
        """
        Initializes class with data
        
        Parameters:
            data_path (str): Path where output files of nbody can be found
            nbody_stdout_files (list): list of file from nbody stdout
        """
        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(logging.DEBUG)
         
        self.data_path = data_path
        # make sure self.data_path ends with "/"
        if (self.data_path and 
            self.data_path[len(self.data_path) -1 ] != "/"):            
            self.data_path += "/"

        self.snap = snap(self.data_path)
        self.cluster = cluster(self.data_path, s=self.snap)
         
        self._data = {}
        
        self._files = {}
        self._files["nb_stdout_files"] = nb_stdout_files if type(nb_stdout_files) == list else [nb_stdout_files]
         
    def __setitem__(self, key, item):
        self._data[key] = item

    def __getitem__(self, key):
        if key not in self._data.keys():
            if key in SUPPORTED_DATA_FILES:
                self.load(key)
            else:
                for k in self._data.keys():
                    if key in self._data[k]:
                        return self._data[k][key]
                for k in SUPPORTED_COLS.keys():
                    if key in SUPPORTED_COLS[k]:
                        self.load(k)
                        return self._data[k][key]

        return self._data[key]

    def __repr__(self):
        return repr(self._data)

    def __len__(self):
        return len(self._data)

    def __delitem__(self, key):
        del self._data[key]
 
    def load(self, *what):
        """
        Loads data into the self._data from files associated with nbody.
        
        If a data type for loading is associated with a file containing more info then just the required
        one, all data associated with a filetype is loaded. E.g. if RLAGR is loaded, also AVMASS, VROT, ...
        will be loaded.
        
        Parameters:        
            what (list): Type of data the function is supposed to load.
                         See SUPPORTED_DATA_LOAD
        """

        if len(what) == 0:
            return self.load(*SUPPORTED_DATA_FILES)

        for load_type in what:
            
            # Handle weird quirks due to global being a keyword in python
            if load_type == "global":
                load_type = "globals"
                self._data["global"] = eval(f"data_files.{load_type}.load('{self.data_path}')")

            elif load_type not in SUPPORTED_DATA_FILES:
                for key in SUPPORTED_COLS.keys():
                    if load_type in SUPPORTED_COLS[key]:
                        data_path = self.data_path
                        if key == "stdout":
                            data_path = [self.data_path + "/" + i for i in self._files['nb_stdout_files']]
                            self._data[key] = eval(f"data_files.{key}.load({data_path})")
                            break
                        else:
                            self._data[key] = eval(f"data_files.{key}.load('{data_path}')")
                            break
                else:
                    raise UnknownDataLoad(f"Couldn't load data for {load_type}")
            else:
                data_path = self.data_path
                if load_type == "stdout":
                    data_path = [self.data_path + "/" + i for i in self._files['nb_stdout_files']]
                    self._data[load_type] = eval(f"data_files.{load_type}.load({data_path})")
                else:
                    self._data[load_type] = eval(f"data_files.{load_type}.load('{data_path}')")


    def calculate_energy_evolution(self):
        if self.snap.shape == (0.3):
            self.snap._load_files()
        self._data["E"] = pd.DataFrame() 
        #self._data["E"] = pd.DataFrame() 

        for i in tqdm(self.snap.reduced.index):
            self.cluster.load(i)

            # TODO: change to pandas version
            if sys.version_info.minor >= 10:
                    self._data["E"].loc[i,["EPOT", "EKIN"]] = [self.cluster.EPOT, self.cluster.EKIN]
            else:
                self._data["E"].__init__(self._data["E"].append(pd.DataFrame(
                                                                 [[ self.cluster.EPOT,
                                                                    self.cluster.EKIN]],
                                                                 index=[i]
                                                            )))
        self._data["E"].sort_index(inplace=True)
    
    def show_mem_usage(self):
        """
        Shows memory usage of nbody class instance.
    
        The memory usage is shown for each data type, sorted from highest memory usage to lowest
        """
        mem_usage = []
        for key in self._data.keys():
            mem_usage += [(key, "%.03f M" % (sys.getsizeof(self._data[key])/1024/1024))]
        mem_usage = sorted(mem_usage, key=lambda item: item[1], reverse=True)
        print("Memory usage:")
        print("Total: %.03f M" % sum([float(i[1].replace(" M", " ")) for i in mem_usage]))
        for data in mem_usage:
            print("%s: %s" % data)
                        
            
    def _analyze_files(self):
        """
        Scans the nbody data dirctory for usable files. Called upon at the end of __init__()
        """
        files = sorted(glob.glob(self.data_path + "*"))
        self.logger.debug("Found data files %s", files)
        
        self.logger.info("Analyzing data files in %s", self.data_path)
        
        for file in tqdm(files):
            for file_type in SUPPORTED_DATA_FILES.keys():
                if re.search(SUPPORTED_DATA_FILES[file_type], file):
                    self._files[file_type] += [file]
                    break            