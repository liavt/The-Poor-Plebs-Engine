/*
Copyright (c) 2016-2019 Liav Turkia

See LICENSE.md for full copyright information
*/
#include <MACE/MACE.h>
#include <random>
#include <ctime>
#include <iostream>

using namespace mc;

class TestComponent: public gfx::Component {
	void init() override {
	}

	bool update() override {
		return false;
	}

	void render() override {}

	void hover() override {
		if( gfx::Input::isKeyDown(gfx::Input::MOUSE_LEFT) ) {
			parent->makeDirty();
		}
	}

	void destroy() override {}

	void clean(gfx::Metrics&) override {
		dynamic_cast<gfx::Image*>(parent)->getTexture().setHue(Color((rand() % 10) / 10.0f, (rand() % 10) / 10.0f, (rand() % 10) / 10.0f, 0.5f));
	}
};

gfx::Image left;
gfx::Image leftBot;
gfx::Image rightTop;
gfx::Image rightBot;

void create(gfx::WindowModule& window) {
	srand((unsigned) time(nullptr));

	const Size elementNum = 10;

	left = gfx::Image(gfx::Texture::getGradient());
	leftBot = gfx::Image(gfx::Texture::getGradient());
	rightTop = gfx::Image(gfx::Texture::getGradient());
	rightBot = gfx::Image(gfx::Texture::getGradient());

	left.setY(0.0f);
	left.setX(-0.5f);
	left.setHeight(0.9f);
	left.setWidth(0.45f);

	leftBot.setX(0.0f);
	leftBot.setY(-0.5f);
	leftBot.setWidth(0.7f);
	leftBot.setHeight(0.4f);

	rightTop.setX(0.5f);
	rightTop.setY(0.5f);
	rightTop.setHeight(0.4f);
	rightTop.setWidth(0.4f);

	rightBot.setHeight(0.4f);
	rightBot.setWidth(0.4f);
	rightBot.setX(0.5f);
	rightBot.setY(-0.5f);

	left.addComponent(std::shared_ptr<gfx::Component>(new TestComponent()));
	leftBot.addComponent(std::shared_ptr<gfx::Component>(new TestComponent()));
	rightTop.addComponent(std::shared_ptr<gfx::Component>(new TestComponent()));
	rightBot.addComponent(std::shared_ptr<gfx::Component>(new TestComponent()));

	left.addChild(leftBot);

	window.addChild(left);
	window.addChild(rightTop);
	window.addChild(rightBot);
}

int main() {
	Instance instance = Instance();
	try {
		gfx::WindowModule::LaunchConfig config = gfx::WindowModule::LaunchConfig(600, 500, "Cleaning Demo");
		config.onCreate = &create;
		config.resizable = true;
		gfx::WindowModule module = gfx::WindowModule(config);
		instance.addModule(module);

		gfx::FPSComponent f = gfx::FPSComponent();
		f.setTickCallback([] (gfx::FPSComponent* com, gfx::Entity*) {
			std::cout << "UPS: " << com->getUpdatesPerSecond() << " FPS: " << com->getFramesPerSecond() << " Frame Time: " << float(1000.0f) / com->getFramesPerSecond() << std::endl;
		});
		module.addComponent(f);

		os::ErrorModule errModule = os::ErrorModule();
		instance.addModule(errModule);

		instance.start();
	} catch( const std::exception& e ) {
		Error::handleError(e, instance);
		return -1;
	}
	return 0;
}
