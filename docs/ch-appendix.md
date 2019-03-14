\appendix

# Appendix

&nbsp;

## One-dimensional Cubic Interpolation

With four sample values, `V0`, `V1`, `V2` and `V3`, cubic interpolation approximates
the curve segment connecting `V1` and `V2`, by using the beginning and ending
slope, the curvature and the rate of curvature change to construct a cubic
polynomial.

The polynomial starts out as:

		a)	f(x) = w3*x^3 + w2*x^2 + w1*x + w0

Where `0 <= x <= 1`, specifying the sample value of the curve segment between
`V1` and `V2` to obtain.

To calculate the coefficients `w[0-3]`, we set out the following conditions:

		b)	f(0)  = V1
		c)	f(1)  = V2
		d)	f'(0) = V1'
		e)	f'(1) = V2'

We obtain `V1'` and `V2'` from the respecting slope triangles:

		f)	V1' = (V2 - V0) / 2
		g)	V2' = (V3 - V1) / 2

With `f) -> d)` and `g) -> e)` we get:

		h)	f'(0) = (V2 - V0) / 2
		i)	f'(1) = (V3 - V1) / 2

The derivation of `f(x)`:

		j)	f'(x)  = 3*w3*x^2 + 2*w2*x + w1

From `0 -> a)`, i.e. `b)`, we obtain `w0` and from `0 -> j)`,
i.e. `h)`, we obtain `w1`. With `w0` and `w1`, we can solve the
linear equation system formed by `c) -> a)` and `e) -> j)`
to obtain `w2` and `w3`.

		c)->a)	  w3 +   w2 + (V2-V0) / 2 + V1 = V2
		e)->j)	3*w3 + 2*w2 + (V2-V0) / 2      = (V3-V1) / 2

With the resulting coefficients:

		w0 = V1									initial value
		w1 = (V2-V0) / 2						initial slope
		w2 = (-V3 + 4*V2 - 5*V1 + 2*V0) / 2		initial curvature
		w3 = (V3 - 3*V2 + 3*V1 - V0) / 2		rate change of curvature

Reducing `a)` to multiplications and additions (eliminating power), we get:

		k)	f(x) = ((w3*x + w2)*x + w1)*x + w0

Based on `V[0-3]`, `w[0-3]` and `k)`, we can now approximate all values of the
curve segment between `V1` and `V2`.

However, for practical resampling applications where only a specific
precision is required, the number of points we need out of the curve
segment can be reduced to an arbitrary amount.
Lets assume we require n equally spread values of the curve segment,
then we can precalculate n sets of `W[0-3][i]`, `i = [0-n]`, coefficients
to speed up the resampling calculation, trading memory for
computational performance. With `w[0-3]` in `a)`

		f(x) = x^3 * (V3 - 3*V2 + 3*V1 - V0) / 2 +
		       x^2 * (-V3 + 4*V2 - 5*V1 + 2*V0) / 2 +
		       x   * (V2 - V0) / 2 +
		              V1

sorted for `V[0-4]`, we have:

		l)	f(x) = V3 * (0.5*x^3 - 0.5*x^2) +
			       V2 * (-1.5*x^3 + 2*x^2 + 0.5*x) +
			       V1 * (1.5*x^3 - 2.5*x^2 + 1) +
			       V0 * (-0.5*x^3 + x^2 - 0.5*x)

With `l)` we can solve `f(x)` for all `x=i/n`, where `i=[0, 1, 2, ..., n]` by
substituting `g(i) = f(i / n)` with

		m)	g(i) = V3 * W3[i] + V2 * W2[i] + V1 * W1[i] + V0 * W0[i]

and using `n` precalculated coefficients `W[0-3]` according to:

		m = i / n
		W3[i] =  0.5*m^3 - 0.5*m^2
		W2[i] = -1.5*m^3 +   2*m^2 + 0.5*m
		W1[i] =  1.5*m^3 - 2.5*m^2         + 1
		W0[i] = -0.5*m^3 +     m^2 - 0.5*m

We now need to setup `W[0-3][0-n]` only once, and are then able to
obtain up to `n` approximation values of the curve segment between
`V1` and `V2` with 4 multiplications and three additions using `m)`,
given `V[0-3]`.
