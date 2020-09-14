# pbr
Random things from physically based rendering that I try

# Path traced PBR

GGX + Smith specular model and Lambertian diffuse model.

<image src="screenies/pbr.png" width="400px"/> <image src="screenies/cornell.jpeg" width="400px"/>

----
A Comparison of various metalness and roughness parameters.

<image src="screenies/pbrm0r1.png" width="400px"/> <image src="screenies/pbrm0r0.png" width="400px"/> <image src="screenies/pbrm1r1.png" width="400px"/> <image src="screenies/pbrm1r0.png" width="400px"/>

| Position     | Roughness | Metalness |
| :------------- | :------------- | :-------- |
| Upper Left       | 0.9       | 0.01 |
| Upper Right | 0.1 | 0.01 |
| Lower Left | 0.9 | 0.9 |
| Lower Right | 0.1 | 0.9 |

# Ray marched PBR

GGX + Smith Specular and a diffuse function from [Shirley(1997)](https://ieeexplore.ieee.org/document/626170)

<image src="screenies/raymarch.jpeg" width="400px"/>
