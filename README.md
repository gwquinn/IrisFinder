# Iris Finder

C++ source code for localizing iris boundaries in standard iris images.
One of the first steps in automated biometric iris recognition is iris boundary localization.
This code is currently in an alpha state. It requires the [OpenCV library](https://opencv.org).

The algorithm used is unpublished but roughly builds upon the approach proposed by [Bendale et. al.](https://www.researchgate.net/publication/230681397_Iris_Segmentation_using_an_Improved_Hough_Transform)

To build, type:
```bash
make

ls examples/img[1-6].png | xargs -Ivar bin/localize var
```
