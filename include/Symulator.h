#include <string>
#include <vector>
#include <list>

#include "raylib.h"

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 450;

class Gate;

class Connector {
public:
    enum class Type {
        IN,
        OUT
    } type;
    Vector2 pos;
    bool value = false;
    Gate* gate;
    Connector(Gate* gate, Vector2 pos, Type type): gate(gate), pos(pos), type(type) {}
    Connector(Vector2 pos, Type type) : gate(nullptr), pos(pos), type(type) {}
    Connector() : gate(nullptr), pos({0, 0}), type(Type::IN) {}
};

class Gate {
public:
    enum class Type {
        NOT,
        AND,
        OR,
        XOR
    } type;
    static constexpr float WIDTH = 75;
    static constexpr float HEIGHT = 30;

    Gate(float x, float y, const char *text, Gate::Type type, bool singleInput = false)
            : rect({x, y, WIDTH, HEIGHT}), text(text), type(type) {
        if (singleInput) {
            inConn.push_back(Connector(this, {x + 5, y + 15}, Connector::Type::IN));
        } else {
            inConn.push_back(Connector(this, {x + 5, y + 5}, Connector::Type::IN));
            inConn.push_back(Connector(this, {x + 5, y + 25}, Connector::Type::IN));
        }
        outConn = Connector(this, {x + 70, y + 15}, Connector::Type::OUT);
    }
    Gate(const Gate* gate)
            : rect(gate->rect), text(gate->text), type(gate->type), inConn(gate->inConn), outConn(gate->outConn) {
        for (auto& in : inConn) {
            in.gate = this;
        }
        outConn.gate = this;
    }
    bool calc() {
        switch (type) {
        case Type::NOT:
            outConn.value = !inConn[0].value;
            break;
        case Type::AND:
            outConn.value = inConn[0].value & inConn[1].value;
            break;
        case Type::OR:
            outConn.value = inConn[0].value | inConn[1].value;
            break;
        case Type::XOR:
            outConn.value = inConn[0].value ^ inConn[1].value;
            break;
        }
        return outConn.value;
    }
    Rectangle rect;
    std::string text;
    bool collide = false;
    std::vector<Connector> inConn;
    Connector outConn;
};

class Input {
  public:
    static constexpr float HEIGHT = 40;
    static constexpr float WIDTH = 40;

    Input(int pos) : pos(pos), y(pos * (HEIGHT + 5) + 40), conn(Connector({45, y + 20}, Connector::Type::OUT)) {}
    int pos;
    float y;
    Connector conn;

    Rectangle getRectangle() { return {0, y, WIDTH, HEIGHT}; }
};

class Output {
  public:
    static constexpr float HEIGHT = 40;

    Output(int pos) : pos(pos), y(pos * (HEIGHT + 5) + 40), conn({SCREEN_WIDTH - 40 - 5, y + 20}, Connector::Type::IN) {}
    int pos;
    float y;
    Connector conn;
};

class Line {
public:
    Connector* start;
    Connector* end;
    Line(Connector* start, Connector* end): start(start), end(end) {}
};

class Symulator {
public:
    enum class State {
        ACTIVE,
        GATE_MOVING,
        LINE_DRAWING
    };

    ~Symulator();
    static void DrawGate(Gate* gate);
    static void DrawInput(Input* in);
    static void DrawOutput(Output* out);

    static void Log(const char *text);

    void CreateGateMenu();
    void CreateInputs();
    void CreateOutputs();

    Gate* CheckGateMenu(const Vector2& pos);
    Gate* CheckGates(const Vector2& pos);
    Input* CheckInputs(const Vector2 &pos);
    Connector* CheckGateEndpoints(Gate *gate, const Vector2 &pos);
    Connector* CheckGatesEndpoints(const Vector2 &pos);
    Connector* CheckInputEndpoints(const Vector2 &pos);
    Connector* CheckOutputEndpoints(const Vector2 &pos);

    bool GateCollide(Gate* gate);

    void AddConnection(Line* line);
    Connector* GetNextConnector(Connector* conn);
    void UpdateConnections();

    void DrawGateMenu();
    void DrawPanel();
    void DrawGates();
    void DrawInputs();
    void DrawOutputs();
    void DrawConnections();

    void Update();
    void Draw();
    int MainLoop();

    State state = State::ACTIVE;

    std::vector<Gate*> gateMenu;
    std::vector<Input*> inputs;
    std::vector<Output*> outputs;
    std::vector<Gate*> gates;
    std::vector<Line*> connections;

    Gate* movingGate;
    Connector* lineStart;
};
