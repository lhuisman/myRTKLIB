# RTKLIB ToDo List

## Binary files in repository

The folders 'dll' and 'lib' contain binary files. 

## Satellite position and APC handling

The precise position returned by `satpos()` refers to the iono-free APC instead of COM for SP3. This comes with the advantage that the satellite antenna PCO is already applied at this stage. Only the correction for the PCV must then be applied for each frequency, which only depends on the Nadir angle. However, this will cause problems for consistent modeling of PCOs for un-combined multi-frequency PPP.

The satellite position for PPP is computed in `ppp.c:pppos()`. From there, the following functions are called:

1. `ephemeris.c:satposs()`

Computes position, clock offset, etc. for each satellite in the observation vector. Calls `ephemeris.c:satpos()` for each satellite.

2. `ephemeris.c:satpos()`

Computes position, clock offset, etc. for single satellite. Uses a switch `opt` to set teh computed positio to either CoM (`opt=0`) or APC (`opt=1`) depending on ephemeris type. Calls `preceph.c:peph2pos()` with `opt=1` when the ephemeris type `EPHOPT_PREC` is selected.

3. `preceph.c:peph2pos()`

Computes the satellite position based on SP3. Here, the reference can be set either to CoM or APC using the `opt` switch. Calls the function `preceph.c:satantoff()` to compute the iono-free PCO.

## Partial support of Bias-SINEX

Not all OSBs in Bias-SINEX files seem to be supported.

## Bias file input in rtkpost_qt

After starting the processing `postpos.c:execses()` first checks if any of the files defined in the main GUI is a bias file. If no biases are found, a DCB file defined under `Options->Files` is read.

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
