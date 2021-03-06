#! /usr/bin/env python
# -*- coding: utf-8 -*-

"""
Compare the current output of nftsim against the stored data for the
configuration files in configs/.

USAGE:
  config_compare.py [options]
  See,
  config_compare.py --help

OPTIONS:
   --conf=filename.conf       : (str)  the configuration file to use.
   --trace='trace.index.name' : (str)  name of the trace to look at.
   --node=0                   : (int)  node to look at (0-based indexing).
   --diff                     : (bool) switch to enable plot type diff.
   --overlay                  : (bool) to enable plot type overlay.
   --help                     : print a help message (autogenerated by argparse).

EXAMPLES:
  #The default configuration file to compare is e-cortical.conf, the default trace
  #is 'propagator.1.phi', the default node index is 0, so to show an overlay
  #and difference (current Vs stored) plot for the first output node of the
  #first propagator's phi for the e-cortical.conf configuration file, from the main
  #nftsim directory just run:
  ./utils/configs_compare.py --diff --overlay

  #To compare 'eirs-eyes-closed.conf', plotting an overlay and diff, of the 3rd
  #output node, with the default for other options, run:
  ./utils/configs_compare.py --conf=example.conf --node=2 --overlay --diff

  #Or just the diff for 'e-cortical.conf' looking at trace 'Pop.2.Q', with the
  #default for other options, run:
  python -i ./utils/configs_compare.py --conf=e-cortical.conf --trace='pop.2.q' --diff
  #NOTE: by launching with 'python -i' we end up at a python command prompt with
  #our data in NF objects named 'current' and 'stored' as well as access to our
  #plotting commands; and trace names on the NF-object are all cast to lower case,
  #so 'Pop.2.Q' becomes 'pop.2.q'.

REQUIRES:
  Recommend using Anaconda on Linux.
  numpy
  matplotlib

.. moduleauthor:: Stuart Knock <>
"""

__version__ = '0.2.0'

# First, import from python standard library
import logging
logging.basicConfig(level=logging.DEBUG)
LOG = logging.getLogger(__name__)
import sys
import os
import gzip
import shutil

# Then standard Python packages
import numpy
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import pyplot

# Then, try importing less reliable packages
try:
    import nftsim
except ImportError:
    LOG.error("Failed to import nftsim.py...")
    raise
###########################################################################

# Function definitions
def plot_overlay(stored, current, trace, node=0):
    fig = pyplot.figure()
    ax = fig.add_subplot(111)
    ax.set_xlabel('time (s)')
    ax.set_ylabel(trace)
    ax.set_title("Overlay for node %s" % stored.nodes[trace][node])
    ax.plot(stored.time, stored.data[trace][:, node], 'b')
    ax.plot(current.time, current.data[trace][:, node], 'r')

def plot_diff(stored, current, trace, node=0):
    fig = pyplot.figure()
    ax = fig.add_subplot(111)
    ax.set_xlabel('time (s)')
    ax.set_ylabel('Diff of %s' % trace)
    ax.set_title("Diff for node %s" % stored.nodes[trace][node])
    diff = current.data[trace][:, node] - stored.data[trace][:, node]
    ax.plot(current.time, diff, 'r')

def plot_surface(trace, timepts):
    raise NotImplementedError

def plot_surface_diff(trace, timepts):
    raise NotImplementedError

def plot_surface_movie(trace):
    raise NotImplementedError

def find_file(filename, path):
    """Find and return the path to file 'filename' in 'path'."""
    for root_dir, _, files in os.walk(path):
        if filename in files:
            return os.path.join(root_dir, filename)

###########################################################################

if __name__ == "__main__":

    #Parse arguments
    import argparse
    PARSER = argparse.ArgumentParser(
        description="Compare the current output of nftsim to stored output.")

    #Configuration file
    PARSER.add_argument('-c', '--conf',
                        default='e-cortical.conf',
                        help="Name of the .conf file you want to check.")

    PARSER.add_argument('--trace',
                        default='propagator.1.phi',
                        help="Name of the trace to look at.")

    PARSER.add_argument('--node',
                        type=int,
                        default=0,
                        help="Node to look at (0-based indexing).")

    PARSER.add_argument('--overlay',
                        action='store_true',
                        default='False',
                        help="Use this flag to turn on overlay plotting.")

    PARSER.add_argument('--diff',
                        action='store_true',
                        default='False',
                        help="Use this flag to turn on diff plotting.")

    PARSER.add_argument('--surface',
                        action='store_true',
                        default='False',
                        help="Use this flag to turn on surface plotting.")

    #version arg
    PARSER.add_argument('--version',
                        action='version',
                        version=__version__,
                        help="Displays version information.")
    #Logging arg
    PARSER.add_argument("--verbosity",
                        type=int,
                        choices=[0, 1, 2, 3],
                        default=2,
                        help="""Set the logging verbosity: 0=ERROR; 1=WARNING;
                                2=INFO; 3=DEBUG.""")
    ARGS = PARSER.parse_args()
    #######################################################################
    #import pdb; pdb.set_trace()

    #Set the logging level
    LOGLEVELMAPPING = ["ERROR", "WARNING", "INFO", "DEBUG"]
    LOG.setLevel(LOGLEVELMAPPING[ARGS.verbosity])
    #######################################################################

    # Main script

    #Identify paths.
    script_path = os.path.dirname(os.path.realpath(__file__))
    nf_path = os.path.abspath(os.path.join(script_path, os.pardir))
    configs_dir = os.path.join(nf_path, 'test', 'numerical')
    test_data_dir = os.path.join(configs_dir, 'data')

    #Configuration file.
    config_filename = os.path.basename(ARGS.conf)
    config_name = os.path.splitext(config_filename)[0]
    config_filepath = find_file(config_filename, configs_dir)
    #config_dir = os.path.dirname(config_filepath)

    #Stored test data from configuration file.
    stored_filename = '%s.output' % config_name
    stored_filepath = find_file(stored_filename, test_data_dir)

    # #Copy the stored data to a temporary file
    # tmp_stored_filepath = '/tmp/nftsim_stored_%s.output' % config_name
    # with gzip.open(stored_filepath, 'rb') as f_in, open(tmp_stored_filepath, 'wb') as f_out:
    #     shutil.copyfileobj(f_in, f_out)

    #Load the stored data into a nftsim object.
    stored = nftsim.NF(stored_filepath)

    #Run the config file with the current version, and load data to a nftsim object.
    current = nftsim.run(config_filepath)

    #If requested, plot overlay of stored and current traces.
    if ARGS.overlay is True:
        plot_overlay(stored, current, ARGS.trace, ARGS.node)

    #If requested, plot overlay of stored and current traces.
    if ARGS.diff is True:
        plot_diff(stored, current, ARGS.trace, ARGS.node)

    pyplot.show()
