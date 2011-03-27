#!/usr/bin/env python
#
#  use the standard messy monolith reader to get some data going.
#
from I3Tray import *

from os.path import expandvars

import os
import sys
from glob import glob

load("libdataclasses")
load("libphys-services")
load("libdataio")

tray = I3Tray()

tray.AddModule("I3Reader", "reader", Filename="one_event.i3")

tray.AddModule("Dump","dump")

tray.AddModule("TrashCan", "the can");

tray.Execute()
tray.Finish()

