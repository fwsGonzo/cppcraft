#include "generator.h"
// console
void (*logText)(char* text) = (void(*)(char*))4254672;

// world
void (*generate)(void* genfunc, int use_border) = (void*)4254304;
void* (*getSector)(int x, int y, int z) = (void*)4254752;
int (*getWorldOffsetX)() = (void*)4255648;
int (*getWorldOffsetZ)() = (void*)4255680;

// interpolation
f32_t (*cosintrp)(f32_t a, f32_t b, f32_t mixrate) = (void*)4211360;
f32_t (*iarray)(f32_t *weights, f32_t x, f32_t y) = (void*)4211616;
f32_t (*cosarray)(f32_t *weights, f32_t x, f32_t y) = (void*)4211440;
f32_t (*cubic)(f32_t *p, f32_t x) = (void*)4211728;
f32_t (*catmull_rom)(f32_t *p, f32_t x) = (void*)4212016;
f32_t (*bicubic)(f32_t *p, f32_t x, f32_t y) = (void*)4211840;
f32_t (*bicubic_catmull)(f32_t *p, f32_t x, f32_t y) = (void*)4212176;
f32_t (*trilin)(f32_t *p, f32_t x, f32_t y, f32_t z) = (void*)4213088;
f64_t (*tri64)(f64_t *p, f64_t x, f64_t y, f64_t z) = (void*)4213376;
f32_t (*costri)(f32_t *p, f32_t x, f32_t y, f32_t z) = (void*)4212352;
f64_t (*costri64)(f64_t *p, f64_t x, f64_t y, f64_t z) = (void*)4212720;
f32_t (*tricubic)(f32_t *p, f32_t x, f32_t y, f32_t z) = (void*)4213664;

// curves
#define cosp(f)    ((1.0 - cos(f * PI)) * 0.5)
#define cubic_hermite(f) f*f*(3.0 - 2.0*f)
#define quintic(f) (f*f*f*(f*(f*6.0-15.0)+10.0))
#define quintic_heavy(f) ( x*x*x*x * ( 25.0 - 48.0 * x + x*x * ( 25.0 - x*x*x*x ) ) )
#define catmull(p, x) ( p[1] + 0.5 * x*( p[2] - p[0] + x*( 2.0 * p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*( 3.0 * (p[1] - p[2]) + p[3] - p[0]) ) ) )

// standard stuff
f32_t (*ramp)(f32_t r, f32_t power) = (void*)4211072;
f64_t (*ramp64d)(f64_t r, f64_t power) = (void*)4211200;
f64_t (*clamp)(f64_t a, f64_t b, f64_t value) = (void*)4210496;

// blocks
void* (*getSectorBlock)(void* sector) = (void*)4254912;
void* (*createSectorBlock)(void* sector) = (void*)4254944;
void (*setsimple)(void* sector, int x, int y, int z, block_t id) = (void*)4228144;
void (*setb)(int x, int y, int z, block_t id, int overwrite, int facing) = (void*)4227136;
void (*setbl)(int x, int y, int z, block* bl, int overwrite) = (void*)4227520;
void (*setblock)(void* sector, int x, int y, int z, block_t id, int overwrite, int facing) = (void*)4227856;
block* (*getb)(int x, int y, int z) = (void*)4226896;
int (*wrapb)(int x, int y, int z) = (void*)4226736;   // returns GEN_FALSE if blocks are out of 'miniworld' bounds

// biomes / flatland data

void* (*getFlatland)(int x, int z) = (void*)4255008;
void (*setTerrain)(void* fland, int bx, int bz, int value) = (void*)4255120;
void (*setColor)(void* fland, int bx, int bz, int clid, cl_rgb* cl) = (void*)4255232;
void (*setColorExt)(void* fland, int bx, int bz, int clid, cl_rgba* cl) = (void*)4255488;
cl_rgb* (*getColor)(void* fland, int bx, int bz, int clid) = (void*)4255312;
int  (*getTerrain)(void* fland, int bx, int bz) = (void*)4255168;
void (*setLevels)(void* fland, int bx, int bz, int, int) = (void*)4255584;

// random functions
f32_t (*iRnd)(int x, int y, int z) = (void*)4215296;
f32_t (*iRnd2)(void* sector, int bx, int by, int bz) = (void*)4216304;
f32_t (*iRnd1)(void* sector, int offset) = (void*)4216448;

// terrain value noise functions
f32_t (*bigRnd)(int wx, int wy, int size) = (void*)4216672;
f32_t (*bigRndCat)(int wx, int wy, int size) = (void*)4217056;

// inGen object functions
void (*ingenAppleTree)(int x, int y, int z, int height) = (void*)4233696;
void (*ingenCactus)(int x, int y, int z, int height) = (void*)4236192;
void (*ingenBigDarkTree)(int x, int y, int z, int height) = (void*)4237184;
void (*ingenJungleTree)(int x, int y, int z, int height) = (void*)4238544;
void (*ingenTreeA)(int x, int y, int z, int height) = (void*)4230944;
void (*ingenPalm)(int x, int y, int z, int height) = (void*)4231488;
void (*ingenPine)(int x, int y, int z, int height) = (void*)4230528;
void (*ingenJungleTreeB)(int x, int y, int z, int h) = (void*)4232304;

