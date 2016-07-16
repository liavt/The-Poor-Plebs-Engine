#include <MC-Network/Network.h>

namespace mc {
	namespace net
	{
		void NetworkModule::init() {
			SDLNet_Init();
		}

		void NetworkModule::tick() {

		}

		void NetworkModule::destroy() {
			SDLNet_Quit();
		}

		std::string NetworkModule::getName() const {
			return "MC-Network";
		}
	}
}