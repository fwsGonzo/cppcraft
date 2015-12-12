#include "../blocks.hpp"

#include "terrains.hpp"
#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../biomegen/biome.hpp"
#include "../processing/postproc.hpp"
#include <glm/gtc/noise.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/math/toolbox.hpp>

#include "../../player.hpp"
#include "../../sector.hpp"
#include "../../particles.hpp"
#include "../../tiles.hpp"
#include "../../sun.hpp"

using namespace glm;
using namespace cppcraft;
using namespace library;
#define sfreq2d(v, n) glm::simplex(glm::vec2(v.x, v.z) * float(n))

namespace terragen
{
	static float getheight_icecap(vec2 p)
	{
		p *= 0.005f;
		float n1 = glm::simplex(p * 0.5f);
		float n2 = glm::simplex(p * 0.15f);
		
		return 0.3 - n1 * 0.05 - n2 * 0.1;
	}
	
	static float getnoise_icecap(vec3 p, float hvalue)
	{
		/*
		p.x *= 0.005;
		p.z *= 0.005;
		float n1 = sfreq2d(p, 0.5);
		float n2 = sfreq2d(p, 0.15);
		
		return p.y - 0.3 + n1 * 0.05 + n2 * 0.1;*/
		return p.y - hvalue;
	}
	
	static void icecap_process(gendata_t* gdata, int x, int z, const int MAX_Y, int zone)
	{
		const int wx = gdata->wx * BLOCKS_XZ + x;
		const int wz = gdata->wz * BLOCKS_XZ + z;
		
		block_t _ICE = db::getb("ice_block");
		
		// count the same block ID until a new one appears
		int counter = BLOCKS_Y-1;
		// count current form of dirt/sand etc.
		int soilCounter = 0;
		// the last block we encountered
		Block lastb = air_block;
		
		// start counting from top (pretend really high)
		int skyLevel    = 0;
		int groundLevel = 0;
		int air = BLOCKS_Y - MAX_Y; // simple _AIR counter
		
		for (int y = MAX_Y-1; y > 0; y--)
		{
			Block& block = gdata->getb(x, y, z);
			
			// we only count primary blocks produced by generator, 
			// which are specifically greensoil & sandbeach
			if (block.getID() == _SOIL || block.getID() == _BEACH)
			{
				soilCounter++;
				
				// making stones under water level has priority!
				if (y < WATERLEVEL && soilCounter > PostProcess::STONE_CONV_UNDER)
				{
					block.setID(_STONE);
				}
				else if (block.getID() != _BEACH)
				{
					// from soil to full-snow
					block.setID(_SNOW);
				}
			}
			else soilCounter = 0;
			
			// check if ultradifferent
			if (block.getID() != lastb.getID())
			{
				if (air > 8)
				{
					///-////////////////////////////////////-///
					///- create objects, and litter crosses -///
					///-////////////////////////////////////-///
					if (block.getID() == _SOIL)
						block.setID(_SNOW);
					
					/// terrain specific objects ///
					// TODO: use poisson disc here
					float rand = randf(wx, y, wz);
					if (rand < 0.1)
					{
						// set some bs winter-cross
					}
				}
				if (air && block.getID() == _WATER)
				{
					block.setID(_ICE);
				}
				// ...
				lastb = block;
			}
			else
			{
				// how many times we've seen the same block on the way down
				counter++;
			}
			
			//
			// -== ore deposition ==-
			//
			if (block.getID() == _STONE)
			{
				PostProcess::try_deposit(gdata, x, y, z);
			} // ore deposition
			
			// check if not air or cross
			if (block.isAir())
			{
				air++;
			}
			else
			{
				air = 0;
				if (skyLevel == 0)
					skyLevel = y+1;
				//if (block.isTransparent() == false)
				if (groundLevel == 0)
					groundLevel = y+1;
			}
			
			// use skylevel to determine when we are below sky
			block.setLight((skyLevel == 0) ? 15 : 0, 0);
		} // y
		
		// set skylevel, groundlevel
		if (groundLevel == 0)
			groundLevel = 1;
		gdata->flatl(x, z).groundLevel = groundLevel;
		if (skyLevel == 256)
			skyLevel = 255;
		gdata->flatl(x, z).skyLevel = skyLevel;
		
	} // PostProcess::run()
	
	void terrain_icecap_init()
	{
		int T_ICECAP = 
		terrains.add("icecap", "Icecap", getheight_icecap, getnoise_icecap);
		Terrain& terrain = terrains[T_ICECAP];
		
		terrain.setFog(glm::vec4(0.5f, 0.6f, 0.7f, 0.7f), 32);
		terrain.on_process = icecap_process;
		
		// snow particle
		int P_SNOW = particleSystem.add("snowflake",
		[] (Particle& p, glm::vec3)
		{
			// slow snow
			p.acc = glm::vec3(0.0f);
			p.spd = glm::vec3(0.0f, -0.05f, 0.0f);
			p.ttl = 180;
		},
		[] (Particle& p, particle_vertex_t& pv)
		{
			pv.size    = 16;
			pv.tileID  = 1 + 1 * tiles.partsX; // (1, 1) = snow particle
			pv.uvscale = 255;
			pv.shiny   = 0;
			
			// determina fade level
			float fade = p.ttl / 32.0f;
			fade = (fade > 1.0f) ? 1.0f : fade;
			// set visibility
			pv.alpha  = fade * 255;
			pv.bright = thesun.getRealtimeDaylight() * 255;
			pv.offsetX = 0;
			pv.offsetY = 0;
			// snow (white + 100% alpha)
			pv.color = 0xFFFFFFFF;
		});
		
		terrain.on_tick = 
		[P_SNOW] (double)
		{
			// every time we tick this piece of shit, we create some SNOW YEEEEEEEEEEEEEE
			for (int i = 0; i < 5; i++)
			{
				// create random position relative to player
				glm::vec3 position(player.pos.x, 0, player.pos.z);
				
				// create particle at skylevel + some value
				Flatland::flatland_t* fs = 
					sectors.flatland_at(position.x, position.z);
				if (fs == nullptr) break;
				
				// use skylevel as particle base height
				position.y = fs->skyLevel;
				// 
				position += glm::vec3(rndNorm(64), 14 + rndNorm(20), rndNorm(64));
				
				// now create particle
				int I = particleSystem.newParticle(position, P_SNOW);
				assert(I >= 0);
			}
		};
		
		// Terrain reddish
		terrain.setColor(Biome::CL_GRASS,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(106, 106, 0, 255);
		});
		// Crosses copy the grass color
		terrain.copyColor(Biome::CL_CROSS, Biome::CL_GRASS);
		// Trees
		terrain.setColor(Biome::CL_TREES, 
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(60, 86, 0, 255);
		});
		// Light-gray stones
		terrain.colors[Biome::CL_STONE] = 
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(180, 180, 180, 255);
		};
	}
}
