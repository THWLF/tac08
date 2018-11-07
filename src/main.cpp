#include <iostream>

#include <SDL2/SDL.h>

#include "hal_core.h"
#include "pico_cart.h"
#include "pico_core.h"
#include "pico_data.h"
#include "pico_script.h"

int safe_main(int argc, char** argv) {
	GFX_Init(512, 512);
	GFX_CreateBackBuffer(128, 128);
	pico_control::init(128, 128);
	pico_data::load_font_data();

	if (argc > 1) {
		pico_cart::load(argv[1]);
	} else {
		std::cerr << "no cart specified" << std::endl;
		return 1;
	}

	bool init = false;

	uint32_t target_ticks = 20;
	uint32_t ticks = 0;

	uint32_t systemFrameCount = 0;
	uint32_t gameFrameCount = 0;
	uint32_t frameTimer = TIME_GetTime_ms();

	while (true) {
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			} else {
				INP_ProcessInputEvents(e);
			}
		} else {
			using namespace pico_api;

			if ((TIME_GetTime_ms() - ticks) > target_ticks) {
				pico_control::set_input_state(INP_GetInputState());
				pico_control::set_mouse_state(INP_GetMouseState());

				if (!init) {
					pico_script::run("_init", true);
					init = true;
				}

				if (!pico_script::run("_update", true)) {
					if (pico_script::run("_update60", true)) {
						target_ticks = 1;
					}
				}
				pico_script::run("_draw", true);

				int buffer_w;
				int buffer_h;
				pico_api::colour_t* buffer = pico_control::get_buffer(buffer_w, buffer_h);
				GFX_CopyBackBuffer(buffer, buffer_w, buffer_h);

				ticks = SDL_GetTicks();
				gameFrameCount++;
			}
			systemFrameCount++;
			GFX_Flip();

			if (TIME_GetElapsedTime_ms(frameTimer) >= 1000) {
				std::cout << gameFrameCount << " " << systemFrameCount << std::endl;

				gameFrameCount = 0;
				systemFrameCount = 0;
				frameTimer = TIME_GetTime_ms();
			}
		}
	}
	GFX_End();
	return 0;
}

int main(int argc, char** argv) {
	try {
		return safe_main(argc, argv);
	} catch (gfx_exception& err) {
		std::cerr << err.what() << std::endl;
	} catch (pico_script::error& err) {
		std::cerr << err.what() << std::endl;
	} catch (pico_cart::error& err) {
		std::cerr << err.what() << std::endl;
	}
}
