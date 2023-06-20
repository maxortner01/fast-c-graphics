#include <iostream>

#include <fcg-cpp/fcg-cpp.hpp>

struct Vertex
{
    float x, y;
};

int main()
{
    // Order... matters?
    // For some reason, putting the machine further down corrupts
    // the memory... Idk WTF is going on here
    fcg::Machine machine;

    fcg::WindowData data = {
        .size = { 1280, 720 },
        .title = "Hello"
    };

    fcg::Window surface(data);

    fcg::GDI gdi(surface, machine);

    /* Print out the devices */
    std::vector<fcg::GraphicsDevice> devices;
    machine.getGraphicsDevices(devices);
    for (const auto& d : devices) std::cout << d.getName() << "\n";

    fcg::BufferLayout layout(machine);
    auto& buffer = layout.createBuffer();

    std::vector<Vertex> vertices = { 
        { 0, 0 },
        { 1, 0 },
        { 1, 1 } 
    };

    buffer.getAttributes().push({ .element_count = 2, .element_size = sizeof(float) });
    buffer.setData(&vertices[0], sizeof(Vertex) * vertices.size());

    fcg::C::FCG_KeepSurfaceOpen((fcg::C::FCG_Surface*)&surface);
}