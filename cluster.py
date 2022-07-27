import numpy as np
import pandas as pd
import logging

from pythonbody.snap import snap
from pythonbody.utils import grav_pot

class cluster(pd.DataFrame):
    def __init__(self,
            data_path: str,
            time: float = None,
            s: snap = None):
        super().__init__(columns=["M", "X1", "X2", "X3", "V1", "V2", "V3"])

        self.data_path = data_path
        self.snap = s
        self.time = None

        if time is not None:
            self.time = time
            self.load(self.time)

    def load(self, time: float):
        if self.snap is None:
            if self.data_path is None:
                logging.error("You need to specify either a snap instance or a datapath")
                return 0
            self.snap = snap(self.data_path)

        self.drop(self.index,inplace=True)
        self.time = time

        data = self.snap.load_cluster(time)
        super().__init__(data)

    @property
    def EKIN(self):
        if not "EKIN" in self.columns:
            self["EKIN"] = 0.5*self["M"]*(self["V1"]**2 + self["V2"]**2 + self["V3"]**2)
        return self["EKIN"].sum()

    @property
    def EPOT(self):
        if not "EPOT" in self.columns:
            self["EPOT"] = grav_pot(self)
        return self["EPOT"].sum()


