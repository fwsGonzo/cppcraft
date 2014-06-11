#ifndef PLAYER_INPUTS_HPP
#define PLAYER_INPUTS_HPP

#include <library/opengl/input.hpp>
#include <cmath>

namespace cppcraft
{
	typedef struct keyconf_t
	{
		/// Keyboard related ///
		int k_forward;
		int k_backward;
		int k_right;
		int k_left;
		
		int k_jump;
		int k_crouch;
		int k_sprint;
		int k_throw;
		int k_inventory;
		
		int k_flying;
		int k_flyup;
		int k_flydown;
		
		/// Mouse related ///
		
		bool alternateMiningButton;
		
		/// Joystick related ///
		bool joy_enabled;
		char joy_index;
		float joy_deadzone;
		float joy_speed;
		
		int joy_button_count;
		unsigned char const* jbuttons;
		int joy_axis_count;
		float const* jaxis;
		
		unsigned char joy_axis_forward;
		unsigned char joy_axis_sidestep;
		unsigned char joy_axis_look_xrot;
		unsigned char joy_axis_look_yrot;
		
		unsigned char joy_axis_mine;
		unsigned char joy_axis_place;
		
		unsigned char joy_btn_jump;
		unsigned char joy_btn_crouch;
		unsigned char joy_btn_sprint;
		
		unsigned char joy_btn_previtem;
		unsigned char joy_btn_nextitem;
		
		unsigned char joy_btn_flying;
		unsigned char joy_btn_exit;
		
	} keyconf_t;
	
	extern keyconf_t keyconf;
	extern library::Input input;
	
	inline float tresholdValue(float x)
	{
		if (std::abs(x) < keyconf.joy_deadzone) return 0.0;
		if (x > 0)
			return (x - keyconf.joy_deadzone) / (1.0 - keyconf.joy_deadzone);
		else
			return (x + keyconf.joy_deadzone) / (1.0 - keyconf.joy_deadzone);
	}
}

#endif
