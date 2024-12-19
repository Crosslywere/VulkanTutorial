#include <Application.h>

class Test : public Application {
public:
	Test() {
		Title = "Test";
		Width = 800;
		Height = 450;
	}

	virtual void OnCreate() override {
	}

	virtual void OnUpdate(float dt) override {
	}

	virtual void OnRender() override {
	}
};

int main(int argc, char** argv) {
	Test app;
	app.Run();
}