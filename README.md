# Transfer Matrix Method (TMM)

The Transfer Matrix Method is a technique for analyzing wave propagation in layered and periodic media, particularly useful for structures where the refractive index varies along one dimension (e.g., Bragg gratings).

## Theory

TMM represents each layer and interface as a matrix that relates forward and backward propagating waves. For an interface between two media with refractive indices $n_1$ and $n_2$:

```math
\begin{bmatrix} E_1 \\ E_1' \end{bmatrix} = T \begin{bmatrix} E_2 \\ E_2' \end{bmatrix}
```

The transfer matrix method consists of two components:

**Propagation Matrix** within a uniform layer of length $L$:

```math
P = \begin{bmatrix} 
e^{i\beta L} & 0 \\ 
0 & e^{-i\beta L} 
\end{bmatrix}
```

**Transfer Matrix** at an index discontinuity:

```math
T_{12} = \begin{bmatrix} 
\frac{n_1+n_2}{2\sqrt{n_1 n_2}} & \frac{n_1-n_2}{2\sqrt{n_1 n_2}} \\ 
\frac{n_1-n_2}{2\sqrt{n_1 n_2}} & \frac{n_1+n_2}{2\sqrt{n_1 n_2}} 
\end{bmatrix}
```

This form is derived from Fresnel coefficients at normal incidence.

## Multi-layer Structures

For structures with multiple layers, transfer matrices are cascaded:

```math
S = P_1 T_{12} P_2 T_{23} \cdots P_N T_N
```

## Periodic Structures Bragg Gratings

For a periodic structure with $N$ periods (alternating between media 1 and 2):

```math
S = (P_1 T_{12} P_2 T_{21})^N
```

The reflection and transmission coefficients are then extracted as:

```math
T = \left|\frac{1}{S_{11}}\right|^2
```

```math
R = \left|\frac{S_{21}}{S_{11}}\right|^2
```