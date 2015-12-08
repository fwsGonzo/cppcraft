#include "compilers.hpp"

#include <library/log.hpp>
#include "columns.hpp"
#include "compiler_scheduler.hpp"
#include "precompiler.hpp"
#include "world.hpp"
#include <mutex>

using namespace library;

namespace cppcraft
{
	// initialize compiler data buffers
	void Compilers::init()
	{
		logger << Log::INFO << "* Initializing compilers" << Log::ENDL;
		columns.init();
	}
	
	// run compilers and try to clear queue, if theres enough time
	void Compilers::run()
	{
		Precomp* precomp;
		while ((precomp = CompilerScheduler::get()) != nullptr)
		{
			int x = precomp->sector.wx - world.getWX();
			int z = precomp->sector.wz - world.getWZ();
			
			// so, what do we do here? I think we just ignore the
			// mesh for (x, z), since there is nothing to do
			if (x >= 0 && z >= 0 && x < sectors.getXZ() && z < sectors.getXZ())
			{
				Column& cv = columns(x, 0, z);
				cv.compile(x, 0, z, precomp);
			}
			
			//////////////////////////
			// delete the precomp and its data
			delete precomp;
			//////////////////////////
		}
		
	} // handleCompilers
	
}
