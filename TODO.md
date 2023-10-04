# RTKLIB ToDo List

## Binary files in repository

The folders 'dll' and 'lib' contain binary files. 

## SP3 satellite position refers to APC

The precise position returned by satpos() is computed in iono-free APC instead of COM for SP3.

## Partial support of Bias-SINEX

Not all OSBs in Bias-SINEX files seem to be supported.

## Missing slot in rtkplot_qt

The program `rtkplot_qt` misses the slot `MenuShapeFileClick()`

```
QObject::connect: No such slot Plot::MenuShapeFileClick() in plotmain.cpp:308
QObject::connect:  (sender name:   'MenuShapeFile')
QObject::connect:  (receiver name: 'Plot')
```

## Pre-fit measurement residuals

The pre-fit measurement residuals in `ppp_res()` are not corrected for by common offset due to receiver clock errors. This requires a unreasonably large editing threshold to keep the observations in.

## Definition of compilation options

The following options (c.f. `rtklib.h`) must be defined at compilation time:

```
ENAGLO 
ENAQZS 
ENAGAL 
ENACMP 
ENAIRN 
NFREQ
NEXOBS
IERS_MODEL
```
The file `RTKLIB.pri` in the root directory controls this universally for all QT applications. For the CUI applications, this must be individually activated in each makefile!
