#include "Particle.h"
#include "util.h"

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Tests
 * util.c/parseInitialConditions
 * util.c/dumpSnapshot
 */
void test_IO()
{
  Particle_Private p[10] = {
    { 11.0, 12.0, 13.0, 14.0, 15.0 },
    { 21.0, 22.0, 23.0, 24.0, 25.0 },
    { 31.0, 32.0, 33.0, 34.0, 35.0 },
    { 41.0, 42.0, 43.0, 44.0, 45.0 },
    { 51.0, 52.0, 53.0, 54.0, 55.0 },
    { 61.0, 62.0, 63.0, 64.0, 65.0 },
    { 71.0, 72.0, 73.0, 74.0, 75.0 },
    { 81.0, 82.0, 83.0, 84.0, 85.0 },
    { 91.0, 92.0, 93.0, 94.0, 95.0 },
    { 101.0, 102.0, 103.0, 104.0, 105.0 }
  };

  FILE *f = initSnapshotFile("./", 0, 10);
  dumpSnapshot(f, 10, p);
  fclose(f);

  f = fopen("0.dat", "r");
  Particle_Private *p2 = parseInitialConditions(f, 3, 4);
  for (int i = 0; i < 4; i++) {
    assert(p[3+i].x = p2[i].x);
    assert(p[3+i].y = p2[i].y);
    assert(p[3+i].m = p2[i].m);
    assert(p[3+i].vx = p2[i].vx);
    assert(p[3+i].vy = p2[i].vy);
  }
  fclose(f);
}

/**
 * tests Particle.c/addInteractions
 */
void test_addInteractions()
{
  Particle_Private earth = { 5.9736e24, 0, 0, 0, 0, 0, 0, DBL_MAX };
  Particle_Private moon = { 7.3477e22, 0, 3.8447e8, 0, 0, 0, 0, DBL_MAX };
  addInteractions(&earth, &moon);
  assert(abs(earth.fx) < 1);
  assert(abs(earth.fy - 1.98056214048e20) < 1e9);
  assert(abs(earth.dmin - 3.8447e8) < 1e8);
  assert(abs(moon.fx) < 1);
  assert(abs(moon.fy + 1.98056214048e20) < 1e9);
  assert(abs(moon.dmin - 3.8447e8) < 1e8);
}

/**
 * tests Particle.c/addAction
 */
void test_addAction()
{
  Particle_Private earth = { 5.9736e24, 0, 0, 0, 0, 0, 0, DBL_MAX };
  Particle_Shared moon = { 7.3477e22, 0, 3.8447e8 };
  addAction(&moon, &earth);
  assert(abs(earth.fx) < 1);
  assert(abs(earth.fy - 1.98056214048e20) < 1e9);
  assert(abs(earth.dmin - 3.8447e8) < 1e8);
}

/**
 * tests Particle.c/dtUpperBound
 */
void test_dtUpperBound()
{
  // This is a case where the initial speed is null, so
  // the triangular inequality is an equality, and the dt
  // computed by dtUpperBound is exactly the time needed for
  // the earth / the moon to travel 0.1 of the distance between them
  // (if the moon was not spinning of course)
  Particle_Private earth = { 5.9736e24, 0, 0, 0, 0, 0, 0, DBL_MAX };
  Particle_Private moon = { 7.3477e22, 0, 3.8447e8, 0, 0, 0, 0, DBL_MAX };
  addInteractions(&earth, &moon);

  double dt = dtUpperBound(&earth, 0.1);
  assert(abs(dt - 1522895.3455) < 1e-4);
  dt = dtUpperBound(&moon, 0.1);
  assert(abs(dt - 168899.241114 < 1e-6));
}

int main(int argc, char**argv)
{
  test_IO();
  test_addInteractions();
  test_addAction();
  test_dtUpperBound();
  printf("All tests passed\n");
}
