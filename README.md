# cppLight2d

This project illustrates light rendering in 2D with C++.

All samples output PNGs with [svpng](https://github.com/miloyip/svpng).

I learn a lot from miloyip's project light2d, and try to do the same thing with C++.

The main idea is similar with milo's, following are the differences:

1. Ray intersection is used instead of ray marching to speed up, so no SDF, only one step in one ray.
2. Only Line and Circle are used as basic shapes. Polygons are generated from lines.
3. more colors ~ hope it beautiful~

an example of reflection

![](reflect.png)

