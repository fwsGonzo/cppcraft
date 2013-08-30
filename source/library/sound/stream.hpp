#ifndef MUSIC_HPP
#define MUSIC_HPP

#include "bass.h"
#include <string>

namespace library
{
	class Stream
	{
	private:
		HSTREAM handle;
		// crossfade related
		float volume;
		double startTime;
		
	public:
		Stream();
		Stream(std::string);
		
		void load( std::string );
		bool destroy();
		
		bool play();
		void stop();
		
		// returns true if the stream is currently active
		bool isPlaying() const noexcept;
		
		// set volume to level (0..1) in time_ms (milliseconds)
		void setVolume(float vol);
	};
}

#endif