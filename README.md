OpenStudio-CONTAM-Utilities
===========================

OpenStudio-based CONTAM Utility Programs

## Description

A set of utility programs that use an input OpenStudio energy model to do various things with [CONTAM]
(http://www.bfrl.nist.gov/IAQanalysis/). Currently, the programs will only build if you have also built a 
[CONTAM-aware version of OpenStudio](https://github.com/jasondegraw/OpenStudio/tree/43144741_CONTAMTranslator).
The utilities are described below. Be warned, however, that any PRJ files that are produced will not contain
sketchpad information. ContamW will sometimes still load the model, but you mileage may vary. The best use
of the models is probably with ContamX via the command line.

## osm2prj

Translate an OpenStudio model into a CONTAM model. If the program can find EnergyPlus simulation results, then
these results will be used to set model temperatures and flow rates. If a weather file can be found, then a
WTH file will be generated. The command line usage is as follows:

    Usage: osm2prj --input-path=./path/to/input.osm
       or: osm2prj input.osm
    Allowed options:
      -f [ --flow ] arg       leakage flow rate per envelope area [m^3/h/m^2]
      -h [ --help ]           print help message and exit
      -i [ --input-path ] arg path to input OSM file
      -l [ --level ] arg      airtightness: Leaky|Average|Tight (default: Average)
      -q [ --quiet ]          suppress progress output

## compinf

Coming soon

## simplefitinf

Coming soon

## epw2wth

Convert an EPW file into the CONTAM WTH format. This is a stand-alone program that uses the same code that osm2prj uses
to do the EPW conversion.

## Building the Programs

The programs are built using CMake (2.8 or newer should probably work). After the first "configure", you'll probably
need to set `OPENSTUDIO_BUILD_DIR` to point to your OpenStudio build, configure again, and then generate.
