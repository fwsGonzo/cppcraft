#include "columns.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include "camera.hpp"
#include "renderconst.hpp"
#include "threading.hpp"
#include "vertex_block.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	const float Column::COLUMN_DEPRESSION = 8.0f;
	
	// all the columns you'll ever need
	Columns columns;
	
	// column compiler accumulation buffer
	vertex_t* column_dump;
	
	void Columns::init()
	{
		logger << Log::INFO << "* Initializing columns" << Log::ENDL;
		
		//////////////////////
		// allocate columns //
		//////////////////////
		int num_columns = Sectors.getXZ() * Sectors.getXZ() * this->height;
		this->columns = 
			new Column[num_columns]();
		
		//////////////////////////////
		// determine if above water //
		//////////////////////////////
		for (int i = 0; i < num_columns; i++)
		{
			int y = i % this->height;
			// the column is above water if the first sector is >= water level
			columns[i].aboveWater = 
				(y * this->sizeSectors * Sector::BLOCKS_Y >= RenderConst::WATER_LEVEL);
			// determine column size (in sectors)
			columns[i].vbodata = new vbodata_t[getSizeInSectors()]();
		}
	}
	Columns::Columns()
	{
		// initialize basic stuff
		//this->height = 1;
		//this->sizeSectors = Sectors.SECTORS_Y / height;
		
		////////////////////////////////////////////////////////
		// allocate temporary datadumps for compiling columns //
		////////////////////////////////////////////////////////
		
		// COLUMNS_SIZE is the number of sectors in a column
		column_dump = 
			new vertex_t[sizeSectors * RenderConst::MAX_FACES_PER_SECTOR * 4];
	}
	Columns::~Columns()
	{
		delete[] column_dump;
		delete[] columns;
	}
	
	Column::Column()
	{
		// initialize VAO to 0, signifying a column without valid GL resources
		this->vao = 0;
		// set initial flags
		this->updated    = false;
		this->renderable = false;
		this->hasdata = false;
	}
	Column::~Column()
	{
		delete[] this->vbodata;
	}
	
	void Column::compile(int x, int y, int z)
	{
		/////////////////////////////////////////////////////////////
		// assemble entire column from vbodata section of a column //
		/////////////////////////////////////////////////////////////
		
		int vboCount = 0;
		vbodata_t* vboList = new vbodata_t[columns.getSizeInSectors()];
		
		for (int sy = columns.getSizeInSectors()-1; sy >= 0; sy--)
		{
			if (vbodata[sy].pcdata != nullptr)
			{
				// COPY the VBO data section
				vboList[vboCount] = vbodata[sy];
				// renderable and consistent, add to queue
				vboCount += 1;
			}
		}
		
		// exit if this column isn't renderable at all
		if (vboCount == 0)
		{
			logger << Log::WARN << "Column::compile(): column was not ready" << Log::ENDL;
			this->updated    = false;
			this->renderable = false;
			delete[] vboList;
			return;
		}
		
		///////////////////////////////////////////////////////
		// find offsets for each shader type, and total size //
		///////////////////////////////////////////////////////
		
		int totalverts[RenderConst::MAX_UNIQUE_SHADERS] = {0};
		
		// go through entire column and find entry points and total bytes
		for (int vy = 0; vy < vboCount; vy++)
		{
			// count vertices
			for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
			{
				// increase by vertices from each path
				totalverts[i] += vboList[vy].vertices[i];
			}
		}
		
		vertex_t* vertoffset[RenderConst::MAX_UNIQUE_SHADERS];
		int vertcount = 0; // dont remove this, needed below
		
		// set the proper offsets into offset(i)
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			vertoffset[i] = (vertex_t*) column_dump + vertcount;
			
			// set column data:
			this->bufferoffset[i] = vertcount;
			this->vertices[i]     = totalverts[i];
			
			vertcount += totalverts[i];
		}
		
		int totalbytes = vertcount * sizeof(vertex_t);
		
		////////////////////////////////////////////////////////
		// loop through each sector in column and memcpy data //
		////////////////////////////////////////////////////////
		
		for (int vindex = 0; vindex < vboCount; vindex++)
		{
			vbodata_t& v = vboList[vindex];
			
			// loop through all vertices in shader path
			for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
			{
				// find count for this shader type
				if (v.vertices[i])
				{
					// macro for (vertex*)[source] + number_of_vertices
					#define m_sourcedata ((vertex_t*) v.pcdata) + v.bufferoffset[i]
					
					// to: column_dump + offset(i) from: pcdata + bufferoffset(i) in vertices
					memcpy( vertoffset[i], m_sourcedata, v.vertices[i] * sizeof(vertex_t) );
					
					vertoffset[i] += v.vertices[i];
				}
			} // shaders
			
			// remove vertex data permanently
			//delete[] v.pcdata;
			
		} // next vbo
		
		delete[] vboList;
		
		///////////////////////////////////
		// generate resources for column //
		///////////////////////////////////
		
		bool updateAttribs = false;
		
		if (this->vao == 0)
		{
			// occlusion culling
			glGenQueries(RenderConst::MAX_UNIQUE_SHADERS, this->occlusion);
			// vertex array object
			glGenVertexArrays(1, &this->vao);
			// vertex and index buffer object
			glGenBuffers(1, &this->vbo);
			
			updateAttribs = true;
		}
		
		// bind vao
		glBindVertexArray(this->vao);
		
		// bind vbo and set to total bytes
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		// upload data
		glBufferData(GL_ARRAY_BUFFER, totalbytes, column_dump, GL_STATIC_DRAW);
		
		if (updateAttribs)
		{
		// attribute pointers
		glVertexAttribPointer(0, 3, GL_SHORT,			GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, x )); // vertex
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_BYTE,			GL_TRUE,  sizeof(vertex_t), (void*) offsetof(vertex_t, nx)); // normal
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 4, GL_SHORT,			GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, u )); // texture
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE,	GL_TRUE,  sizeof(vertex_t), (void*) offsetof(vertex_t, biome)); // biome color
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE,	GL_TRUE,  sizeof(vertex_t), (void*) offsetof(vertex_t, c));  // shadow and brightness
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE,	GL_TRUE,  sizeof(vertex_t), (void*) (offsetof(vertex_t, c) + 4)); // torchlight color
		glEnableVertexAttribArray(5);
		}
		
		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			throw std::string("Column::compile(): OpenGL error after ending compiler");
		}
		#endif
		
		if (camera.getFrustum().column(x * Sector::BLOCKS_XZ + Sector::BLOCKS_XZ / 2,
								z * Sector::BLOCKS_XZ + Sector::BLOCKS_XZ / 2,
								y * columns.getSizeInSectors() * Sector::BLOCKS_Y,
								columns.getSizeInSectors() * Sector::BLOCKS_Y, 
								Sector::BLOCKS_XZ / 2))
		{
			// update render list
			camera.needsupd = true;
			
			if (this->renderable == false)
			{
				// make sure it is added to renderq
				camera.recalc = true;
				this->pos.y = -COLUMN_DEPRESSION;
			}
		}
		
		// set as renderable, 
		this->renderable = true;
		// and no longer needing update
		this->updated = false;
		// the vbo has data stored in gpu
		this->hasdata = true;
		
		// reset occluded state
		for (size_t i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
			this->occluded[i] = 0;
	}
	
}
