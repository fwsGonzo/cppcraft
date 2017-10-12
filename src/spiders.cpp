#include "spiders.hpp"

#include "generator.hpp"
#include "lighting.hpp"
#include "player.hpp"
#include "player_physics.hpp"
#include "sectors.hpp"

namespace cppcraft
{
	// _AIR block with max lighting
	Block air_block(_AIR);

	// wrap absolute position
	Sector* Spiders::spiderwrap(int& bx, int& by, int& bz)
	{
		int fx = bx >> Sector::BLOCKS_XZ_SH;
		int fz = bz >> Sector::BLOCKS_XZ_SH;

		if (fx < 0 || fx >= sectors.getXZ() ||
			fz < 0 || fz >= sectors.getXZ() ||
			by < 0 || by >= BLOCKS_Y)
				return nullptr;

		bx &= (Sector::BLOCKS_XZ-1);
		bz &= (Sector::BLOCKS_XZ-1);
		return &sectors(fx, fz);
	}

	// wrap position relative to sector
	Sector* Spiders::spiderwrap(Sector& s, int& bx, int& by, int& bz)
	{
		int fx = s.getX() + (bx >> Sector::BLOCKS_XZ_SH);
		int fz = s.getZ() + (bz >> Sector::BLOCKS_XZ_SH);

		if (fx < 0 || fx >= sectors.getXZ() ||
			fz < 0 || fz >= sectors.getXZ() ||
			by < 0 || by >= BLOCKS_Y)
				return nullptr;

		bx &= (Sector::BLOCKS_XZ-1);
		bz &= (Sector::BLOCKS_XZ-1);
		return &sectors(fx, fz);
	}

	Block& Spiders::getBlock(int x, int y, int z)
	{
		Sector* ptr = spiderwrap(x, y, z);
		if (ptr) {
			return ptr[0](x, y, z);
		}
		return air_block;
	}

	Block& Spiders::getBlock(Sector& s, int x, int y, int z)
	{
		Sector* ptr = spiderwrap(s, x, y, z);
		if (ptr) {
			return ptr[0](x, y, z);
		}
		return air_block;
	}

	Block& Spiders::getBlock(float x, float y, float z, float size)
	{
		// make damn sure!
		if (y < 0.0f) return air_block;
		if (y >= BLOCKS_Y) return air_block;

		int   by = int(y);  // integral
		float fy = y - by;  // fractional

		float dx, dz;
		for (dz = z-size; dz <= z+size; dz += size)
		for (dx = x-size; dx <= x+size; dx += size)
		{
			Block& b = getBlock(int(dx), by, int(dz));
			if (b.getID())
			{
				float fx = dx - int(dx);
				float fz = dz - int(dz);
				if (b.physicalHitbox3D(b, fx, fy, fz))
					return b;
			}
		}
		return air_block;
	}

	Block Spiders::testArea(float x, float y, float z)
	{
		return getBlock(x, y, z, PlayerPhysics::PLAYER_SIZE);
	}

	Block Spiders::testAreaEx(float x, float y, float z)
	{
		// make damn sure!
		if (y < 0.0) return air_block;
		if (y >= BLOCKS_Y) return air_block;

		int   by = int(y);  // integral
		float fy = y - by;  // fractional
		const float PLAYER_SIZE = PlayerPhysics::PLAYER_SIZE;

		double dx, dz;
		for (dz = z-PLAYER_SIZE; dz <= z+PLAYER_SIZE; dz += PLAYER_SIZE)
		for (dx = x-PLAYER_SIZE; dx <= x+PLAYER_SIZE; dx += PLAYER_SIZE)
		{
			const Block& block = getBlock(int(dx), by, int(dz));
			if (block.getID() != _AIR)
			{
				float fx = dx - int(dx);
				float fz = dz - int(dz);
				if (block.physicalHitbox3D(block, fx, fy, fz))
				{
					if (block.blocksMovement())
						return block;
				}
			} // not air
		} // 3x3 test grid

		return air_block;
	}

	light_value_t Spiders::getLightNow(float x, float y, float z)
	{
		if (y <= 0.0f) return 0; // mega-dark
		if (y >= BLOCKS_Y) return BLOCKS_Y-1;

		int ix = x, iy = y, iz = z;
		Sector* sector = Spiders::spiderwrap(ix, iy, iz);
		// not really cause to exit, since torchlights can still affect someone barely outside of world,
		// but whatever, we exit when out of bounds
		if (sector == nullptr) return 0;
		if (sector->generated() == false) return 0;

		// return calculated shadows & lighting
		return Lighting::lightValue((*sector)(ix, iy, iz));
	}

}