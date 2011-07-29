// Copyright 2011 Michael E. Stillman

// Routines for Chinese remaindering and rational reconstruction

#ifndef _cra_hpp_
#define _cra_hpp_

#include "engine.h"
// engine.h defines rawRingElementCRA, rawMatrixCRA

#include "matrix.hpp"

class ChineseRemainder {
public:
  static void CRA0(mpz_t a, mpz_t b, mpz_t um, mpz_t vn, mpz_t mn, mpz_t result);
  // computes using precomputed multipliers, the unique integer 'result' < m*n s.t.
  // result == a mod m, and == b mod n.
  // 1 = u*m + v*n, and um = u*m, vn = v*n, mn = m*n

  static bool computeMultipliers(mpz_t m, mpz_t n, mpz_t result_um, mpz_t result_vn, mpz_t result_mn);
  // computes the multipliers um, vn, mn used in CRA0

  static ring_elem CRA(const PolyRing *R, const ring_elem f, const ring_elem g, mpz_t um, mpz_t vn, mpz_t mn);
  // does the vector combination without error checking and with precomputed multipliers
  
  static vec CRA(const PolyRing *R, vec f, vec g, mpz_t um, mpz_t vn, mpz_t mn);
  // does the vector combination without error checking and with precomputed multipliers
  
  static Matrix * CRA(const Matrix *f, const Matrix *g, mpz_t um, mpz_t vn, mpz_t mn);
  // does the matrix combination without error checking and with precomputed multipliers
  
  static RingElement * CRA(const RingElement *f, const RingElement *g,  mpz_t um, mpz_t vn, mpz_t mn);
  // does the ring element combination without error checking and with precomputed multipliers

  



  static ring_elem CRA(const PolyRing *R, ring_elem f, ring_elem g, mpz_t m, mpz_t n);
  // Assumption: f and g are in a poly ring whose coeff ring is ZZ

  static ring_elem reconstruct(const PolynomialRing *RQ, const Ring *R, ring_elem f, mpz_t m);
  // f should be an element in the polynomial ring R (over ZZ).
  // RQ should be the same ring as R, but with rational coefficients

  static RingElement * reconstruct(const Ring *RQ, const RingElement *f, mpz_t m);
  // f should be an element in the polynomial ring R (over ZZ).
  // RQ should be the same ring as R, but with rational coefficients

  static Matrix * reconstruct(const Ring *RQ, const Matrix *f, mpz_t m);
  // f should be an element in the polynomial ring R (over ZZ).
  // RQ should be the same ring as R, but with rational coefficients
};


#endif