#ifndef __VECTOR_H__
#define __VECTOR_H__

/**
 * A real-valued 2-dimensional vector.
 * Positive x is towards the right; positive y is towards the top.
 * vector_t is defined here instead of vector.c because it is passed *by value*.
 */
typedef struct vector {
    double x;
    double y;
} vector_t;

/**
 * The zero vector, i.e. (0, 0).
 * "extern" declares this global variable without allocating memory for it.
 * You will need to define "const vector_t VEC_ZERO = ..." in vector.c.
 */
extern const vector_t VEC_ZERO;

/**
 * Common basis vectors.
 */
extern const vector_t E1;
extern const vector_t E2;
extern const vector_t E3;

/**
 * Adds two vectors.
 * Performs the usual componentwise vector sum.
 *
 * @param v1 the first vector
 * @param v2 the second vector
 * @return v1 + v2
 */
vector_t vec_add(vector_t v1, vector_t v2);

/**
 * Subtracts two vectors.
 * Performs the usual componentwise vector difference.
 * The result points from v2 to v1.
 *
 * @param v1 the first vector
 * @param v2 the second vector
 * @return v1 - v2
 */
vector_t vec_subtract(vector_t v1, vector_t v2);

/**
 * Computes the additive inverse a vector.
 * This is equivalent to multiplying by -1.
 *
 * @param v the vector whose inverse to compute
 * @return -v
 */
vector_t vec_negate(vector_t v);

/**
 * Multiplies a vector by a scalar.
 * Performs the usual componentwise product.
 *
 * @param scalar the number to multiply the vector by
 * @param v the vector to scale
 * @return scalar * v
 */
vector_t vec_multiply(double scalar, vector_t v);

/**
 * Computes the dot product of two vectors.
 * See https://en.wikipedia.org/wiki/Dot_product#Algebraic_definition.
 *
 * @param v1 the first vector
 * @param v2 the second vector
 * @return v1 . v2
 */
double vec_dot(vector_t v1, vector_t v2);

/**
 * Computes the cross product of two vectors,
 * which lies along the z-axis.
 * See https://en.wikipedia.org/wiki/Cross_product#Computing_the_cross_product.
 *
 * @param v1 the first vector
 * @param v2 the second vector
 * @return the z-component of v1 x v2
 */
double vec_cross(vector_t v1, vector_t v2);

/**
 * Returns the magnitude (L^2-Norm) of the given vector.
 */
double vec_magnitude(vector_t v);

/**
 * Returns the unit vector pointing in the direction of 'v'.
 */
vector_t vec_normalize(vector_t v);

/**
 * Rotates a vector by an angle around (0, 0).
 * The angle is given in radians.
 * Positive angles are counterclockwise, according to the right hand rule.
 * See https://en.wikipedia.org/wiki/Rotation_matrix.
 * (You can derive this matrix by noticing that rotation by a fixed angle
 * is linear and then computing what it does to (1, 0) and (0, 1).)
 *
 * @param v the vector to rotate
 * @param angle the angle to rotate the vector
 * @return v rotated by the given angle
 */
vector_t vec_rotate(vector_t v, double angle);

/**
 * Rotate relative to point.
 */
vector_t vec_rotate_relative(vector_t v, double angle, vector_t point);

/**
 * Copies the given vector.
 */
vector_t *vec_p_copy(vector_t *v);

/**
 * calculates the angle (in radians) between the two vectors assuming they
 * form a triangle point shape, with point_around being the place the two
 * vectors touch
 */
double vec_angle_between(vector_t v1, vector_t v2, vector_t point_around);

vector_t *vec_parse_str(const char *str);

// Vector free is normal free.

#endif // #ifndef __VECTOR_H__
