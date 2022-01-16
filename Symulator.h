#include <vector>
#include <string>

#include "raylib.h"

class Gate {
public:
    static constexpr float WIDTH = 75;
    static constexpr float HEIGHT = 30;

    Gate(float x, float y, const char *text)
            : rect({x, y, WIDTH, HEIGHT}), text(text) {}
    Gate(const Gate* gate)
            : rect(gate->rect), text(gate->text) {}
    Rectangle rect;
    std::string text;
};

class Input {
public:
    static constexpr float HEIGHT = 40;
    static constexpr float WIDTH = 40;

    Input(int pos) : pos(pos), y(pos * (Input::HEIGHT + 5)) {
    }
    bool value = 0;
    int pos;
    float y;
    Rectangle getRectangle() { return {0, y + 40 , WIDTH, HEIGHT}; }
};

class Output {
public:
    static constexpr float HEIGHT = 40;

    Output(int pos) : pos(pos) {}
    bool value = 0;
    int pos;
};

class Symulator {
public:
    enum class State {
        ACTIVE,
        GATE_MOVING
    };

    ~Symulator();
    static void DrawGate(Gate* gate);
    static void DrawInput(Input *in);
    static void DrawOutput(Output *out);

    static void Log(const char *text);

    void CreateGateMenu();
    void CreateInputs();
    void CreateOutputs();

    Gate* CheckGateMenu(const Vector2& pos);
    Gate* CheckGates(const Vector2& pos);
    Input* CheckInputs(const Vector2 &pos);

    void DrawGateMenu();
    void DrawPanel();
    void DrawGates();
    void DrawInputs();
    void DrawOutputs();

    void Update();
    void Draw();
    int MainLoop();

    static constexpr int SCREEN_WIDTH = 800;
    static constexpr int SCREEN_HEIGHT = 450;

    State state = State::ACTIVE;

    std::vector<Gate*> gateMenu;
    std::vector<Input*> inputs;
    std::vector<Output*> outputs;
    std::vector<Gate *> gates;

    Gate* movingGate;
};
