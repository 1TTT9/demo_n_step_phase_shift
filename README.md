# demo_n_step_phase_shift


![alt text](https://github.com/1TTT9/demo_n_step_phase_shift/blob/main/images/demo.png?raw=true)

![alt text](https://github.com/1TTT9/demo_n_step_phase_shift/blob/main/images/demo_4x.png?raw=true)

A C++ implementation of n step phase shift algorithm for depth construction of structure-lighted images

The structure lighted technique use n-step phase algorithm for reconstructing 3D depth images from 2D images and projected strips.
It is known as trigonometric triangulation: using several shifts(3, often) and project the patterns to the object for scan, 
and calculate the distance of every point in the field of view. 

I also add 4-step algorithm on the basis of the previous codes and tested the results of depth calculation, ~~however, it seems that the outcome of 3-step looked better for surfacial defect task.~~
The time cost is roughtly 6.12-6.27(s) on my core i71165 labtop, and no significant difererence between 4-step and 3-step.

## updated 

### 2022-11-15
  - fixed mis-use of convertTo function

### 2022-11-11
  - fixed cropping problem (replace corp with clone)
  - ouptut float depth map
  - reconstruct 3d by detph map

### 2022-11-10
  - fixed mis-use of neighbor points in sqdist function
  - added comment for crop operation

### 2022-11-09
  - fixed theta value calculation problem.
  - fixed k_step input problem.

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



