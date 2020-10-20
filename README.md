CUDA Path Tracer
================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Project 3**

* Thy (Tea) Tran 
  * [LinkedIn](https://www.linkedin.com/in/thy-tran-97a30b148/), [personal website](https://tatran5.github.io/), [email](thytran316@outlook.com)
* Tested on: Windows 10, i7-8750H @ 2.20GHz 22GB, GTX 1070

### Denoiser
![](img/cornell_ceiling_light_10_iterations_128x128.png)


![With denoiser (10 iterations, 128x128 filter](img/cornell_ceiling_light_10_iterations_128x128.png)

![With denoiser (10 iterations, 64x64 filter](img/cornell_ceiling_light_10_iterations_64x64.png)

![With denoiser (10 iterations, 32x32 filter](img/cornell_ceiling_light_10_iterations_32x32.png

![With denoiser (10 iterations, 16x16 filter](img/cornell_ceiling_light_10_iterations_16x16.png)

![With denoiser (10 iterations, 5x5 filter](img/cornell_ceiling_light_10_iterations_5x5.png)

![No denoiser (10 iterations)](img/cornell_ceiling_light_10_iterations.png)

The denoiser in this project is based on the paper "Edge-Avoiding A-Trous Wavelet Transform for fast Global Illumination Filtering," by Dammertz, Sewtz, Hanika, and Lensch. You can find [the paper here](https://jo.dreggn.org/home/2010_atrous.pdf) The project also uses geometry buffers (G-buffers) to guid a smoothing filter.

Often times, a pure path tracer will result in noisy (or grainy) renders, like the last image above, which can be improved by applying a denoiser. A denoiser can help create a smoother appearance in the path trace image by blurring pixels for each pixel. 

However, if only a kernel is applied to smooth out the image, important features such as edges of the image might be blurred as well. Hence, we need to take into account the normals, positions (distance), current colors of the current pixel and neighbor pixels to preserve crucial details. 

Another concern is a huge drag in performance if the blur filter size gets larger. Hence, instead of constructing a big filter corresponding to user's input, we can apply the filter on an image sparsely through multiple passes. The example [here](https://onedrive.live.com/view.aspx?resid=A6B78147D66DD722!95296&ithint=file%2cpptx&authkey=!AI_kS-xxETawwBw) helps explaining why it is more efficient to apply the filter sparsely under multipass. Basically, without the sparse application of the filter,for a 16x16 width/filter, there are 256 pixel reads for each pixel blurred. However, if A-Trous was applied, there are only 75 pixel reads per pixel blurred.

### Performance analysis
![](img/denoiserRuntime.png)

As the filter size increases, there is a slight additional runtime. A larger filter size implies that for each pixel in the image, we have to consider more surrounding pixels. 

If this project is built from scratch again, I would not set up g-buffers the way they are currently..