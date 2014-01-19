#include "render_gui_inventory.hpp"

#include <library/log.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/math/matrix.hpp>
#include <library/opengl/opengl.hpp>
#include "blocks.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	GUIInventory quickbarItems;
	static const double PI = 4 * atan(1);
	
	std::vector<GUIInventory::inventory_t> transformedCube;
	
	void GUIRenderer::initInventoryRenderer()
	{
		// pre-transform cube
		vec3 GUI_cube[12] = 
		{
			vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5,  0.5),
			vec3(-0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5,  0.5, -0.5),
			vec3( 0.5, -0.5, -0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5, -0.5,  0.5)
		};
		
		// rotate cube and invert the Y-axis
		mat4 scale(1, -1, 1);
		mat4 matrot = rotationMatrix(PI / 4, -PI / 4, 0);
		// turn the cube upside down because this coordinate system
		// has the positive Y-axis pointing downwards
		matrot = scale * matrot;
		
		for (int vert = 0; vert < 12; vert++)
		{
			GUI_cube[vert] = matrot * GUI_cube[vert];
		}
		
		float GUIcube_tex[24] =
		{
			0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0,
			0.0, 1.0,  0.0, 0.0,  1.0, 0.0,  1.0, 1.0,
			1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0,
		};
		
		// create pre-transformed cube mesh
		
		for (int i = 0; i < 12; i++)
		{
			int face = (i / 4) * 2;
			
			vec3& v = GUI_cube[i];
			
			float tu = GUIcube_tex[i * 2 + 0];
			float tv = GUIcube_tex[i * 2 + 1];
			
			unsigned int color;
			if (face == 0) color = BGRA8(0, 0, 0,    0);
			if (face == 2) color = BGRA8(0, 0, 0,   20);
			if (face == 4) color = BGRA8(0, 0, 0,   64);
			
			transformedCube.emplace_back(v.x, v.y, v.z,  tu, tv, (float)face,  color);
		}
		
	}
	
	void GUIRenderer::renderQuickbarItems(library::mat4& ortho, double frameCounter)
	{
		float width = this->width * 0.4;
		float height = width / 8;
		float posx = (this->width - width) * 0.5;
		float posy = this->height - height;
		
		posx += width * 0.025;
		posy += width * 0.025;
		
		float size   = width * 0.075;
		float stride = width * 0.1095;
		
		if (inventory.isChanged())
		{
			// inventory has no longer changed
			inventory.setChanged(false);
			
			// recreate meshes
			quickbarItems.clear();
			
			for (int x = 0; x < inventory.getWidth(); x++)
			{
				InventoryItem& itm = inventory(x, menu.quickbarY);
				quickbarItems.emit(itm, posx + x * stride, posy, size);
			}
			quickbarItems.upload();
		}
		
		// render inventory
		quickbarItems.render(ortho);
	}
	
	void GUIInventory::clear()
	{
		this->blockTiles.clear();
		this->itemTiles.clear();
	}
	
	int GUIInventory::emit(InventoryItem& itm, float x, float y, float size)
	{
		if (itm.isItem())
		{
			// texture tile id
			float tile = items.tileByID(itm.getID());
			
			return quickbarItems.emitQuad(itm, x, y, size, tile);
		}
		else if (itm.isBlock())
		{
			// some blocks can be represented by quads
			if (itm.getID() == _LADDER || isCross(itm.getID()))
			{
				return quickbarItems.emitBlockQuad(itm, x, y, size);
			}
			// presentable rotated blocks
			return quickbarItems.emitBlock(itm, x, y, size * 0.8);
		}
		return 0;
	}
	
	int GUIInventory::emitQuad(InventoryItem& itm, float x, float y, float size, float tile)
	{
		// no item, no texture!
		if (itm.getID() == IT_NONE) return 0;
		// no count, no item!
		if (itm.getCount() == 0) return 0;
		// create single quad
		itemTiles.emplace_back(
			x,        y + size, 0,   0, 0, tile,   BGRA8(255, 255, 255,   0) );
		itemTiles.emplace_back(
			x + size, y + size, 0,   1, 0, tile,   BGRA8(255, 255, 255,   0) );
		itemTiles.emplace_back(
			x + size, y,        0,   1, 1, tile,   BGRA8(255, 255, 255,   0) );
		itemTiles.emplace_back(
			x,        y,        0,   0, 1, tile,   BGRA8(255, 255, 255, 128) );
		
		return 4;
	}
	
	int GUIInventory::emitBlockQuad(InventoryItem& itm, float x, float y, float size)
	{
		// face value is "as if" front
		float tile = Block::cubeFaceById(itm.getID(), 0, 0);
		// create single quad
		blockTiles.emplace_back(
			x,        y + size, 0,   0, 0, tile,   BGRA8(255, 255, 255,   0) );
		blockTiles.emplace_back(
			x + size, y + size, 0,   1, 0, tile,   BGRA8(255, 255, 255,   0) );
		blockTiles.emplace_back(
			x + size, y,        0,   1, 1, tile,   BGRA8(255, 255, 255,   0) );
		blockTiles.emplace_back(
			x,        y,        0,   0, 1, tile,   BGRA8(255, 255, 255, 128) );
		return 4;
	}
	int GUIInventory::emitBlock(InventoryItem& itm, float x, float y, float size)
	{
		vec3 offset = vec3(x, y, -1) + vec3(size, size, 0) * 0.6;
		
		for (size_t i = 0; i < transformedCube.size(); i++)
		{
			inventory_t& vertex = transformedCube[i];
			
			// move cube to the right position, and scale it down to size
			vec3 v(vertex.x, vertex.y, vertex.z);
			v = offset + v * size;
			
			// face value is located in vertex.w
			float tw = Block::cubeFaceById(itm.getID(), vertex.w, 3);
			
			blockTiles.emplace_back(v.x, v.y, v.z,  vertex.u, vertex.v, tw,  vertex.color);
		}
		return 12;
	}
	
	void GUIInventory::upload()
	{
		int items  = itemTiles.size();
		int blocks = blockTiles.size();
		
		if (items + blocks == 0) return;
		
		// add blocks to the end of items
		if (blocks)
		{
			itemTiles.insert(itemTiles.end(), blockTiles.begin(), blockTiles.end());
		}
		
		/// upload blocks & items ///
		if (itemsVAO.isGood() == false)
		{
			itemsVAO.begin(sizeof(inventory_t), itemTiles.size(), itemTiles.data());
			itemsVAO.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(inventory_t, x));
			itemsVAO.attrib(1, 3, GL_FLOAT, GL_FALSE, offsetof(inventory_t, u));
			itemsVAO.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(inventory_t, color));
			itemsVAO.end();
		}
		else
		{
			itemsVAO.begin(sizeof(inventory_t), itemTiles.size(), itemTiles.data());
			itemsVAO.end();
		}
	}
	
	void GUIInventory::render(mat4& ortho)
	{
		int total  = itemTiles.size();
		
		// nothing to do here with no items or blocks
		if (total == 0) return;
		
		int blocks = blockTiles.size();
		int items  = total - blocks;
		
		/// render all menu items ///
		shaderman[Shaderman::MENUITEM].bind();
		shaderman[Shaderman::MENUITEM].sendMatrix("mvp", ortho);
		
		if (items)
		{
			// items texture
			textureman.bind(0, Textureman::T_ITEMS);
			// render items
			itemsVAO.render(GL_QUADS, 0, items);
		}
		if (blocks)
		{
			// blocks texture
			textureman.bind(0, Textureman::T_DIFFUSE);
			// render blocks
			itemsVAO.render(GL_QUADS, items, blocks);
		}
	} // inventory.render
	
}
