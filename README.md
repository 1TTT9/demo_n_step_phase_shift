# demo_n_step_phase_shift
========================

A C++ implementation of n step phase shift algorithm for depth construction of structure-lighted images

The structure lighted technique use n-step phase algorithm for reconstructing 3D depth images from 2D images and projected strips.
It is known as trigonometric triangulation: using several shifts(3, often) and project the patterns to the object for scan, 
and calculate the distance of every point in the field of view. 

I also add 4-step algorithm on the basis of the previous codes and tested the results of depth calculation, however, 
it seems that the outcome of 3-step looked better for surfacial defect task.


## benefit 
  - fast scan (in 2~10 second*, depending on how many steps you choose)
  - larger scanning area
  - offer quite good resolution

## downside
 - sensitive to lighting condition (e.g., outdoor)
 

## Requirement
  - Opencv (I use 3.1.0)
  - OS: Windows
  

## Note
 For the original codes repo, please refer [this repo](https://github.com/phreax/structured_light) 



