#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "implot.h"
#include <vector>
#include <cmath>
#include <string> 
#include "physics.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void drawArc(sf::RenderWindow& window, sf::Vector2f center, float radius) {
    const int segments = 40;
    sf::VertexArray arc(sf::LineStrip, segments + 1);

    for (int i = 0; i <= segments; i++) {
        float angle = M_PI * i / segments;
        float x = center.x + radius * std::cos(angle);
        float y = center.y + radius * std::sin(angle);
        arc[i].position = {x, y};
    }
    window.draw(arc);
}

void drawVector(sf::RenderWindow& window, sf::Vector2f origin, sf::Vector2f vec) {
    sf::Vector2f end = origin + vec;

    sf::Vertex line[] = {
        sf::Vertex(origin),
        sf::Vertex(end)
    };
    window.draw(line, 2, sf::Lines);

    float mag = std::sqrt(vec.x * vec.x + vec.y * vec.y);
    if (mag == 0) return;

    sf::Vector2f dir = vec / mag;
    sf::Vector2f perp = {-dir.y, dir.x};
    float size = 8;

    sf::Vector2f p1 = end - dir * size + perp * size;
    sf::Vector2f p2 = end - dir * size - perp * size;

    sf::Vertex arrow[] = {
        sf::Vertex(end),
        sf::Vertex(p1),
        sf::Vertex(end),
        sf::Vertex(p2)
    };
    window.draw(arrow, 4, sf::Lines);
}

void drawLabel(sf::RenderWindow& window, sf::Font& font, const std::string& text, 
               sf::Vector2f centerPos, unsigned int size = 16) {

    sf::Text t;
    t.setFont(font);
    t.setString(text);
    t.setCharacterSize(size);
    
    t.setFillColor(sf::Color::White);
    t.setOutlineColor(sf::Color::Black);
    t.setOutlineThickness(2.0f); 

    sf::FloatRect textBounds = t.getLocalBounds();
    
    t.setOrigin(textBounds.left + textBounds.width / 2.0f,
                textBounds.top  + textBounds.height / 2.0f);

    t.setPosition(centerPos);

    window.draw(t);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 650), "Atwood Machine Simulator");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) {
        return -1;
    }
    ImPlot::CreateContext();

    float m1 = 2.0f, m2 = 1.0f;
    float g = 9.8f;
    float a = acceleration(m1, m2, g);
    float T = (2 * m1 * m2 * g) / (m1 + m2); 

    float y0 = 0, v0 = 0;
    float scale = 100;

    float pulleyX = 335;
    float pulleyY = 120;
    float radius = 30;

    sf::CircleShape pulley(radius);
    pulley.setFillColor(sf::Color::Transparent);
    pulley.setOutlineThickness(3);
    pulley.setOutlineColor(sf::Color::White);
    pulley.setPosition(pulleyX - radius, pulleyY - radius);

    float baseSize = 30.0f;
    sf::RectangleShape block1({baseSize * m1, baseSize * m1});
    sf::RectangleShape block2({baseSize * m2, baseSize * m2});
    
    block1.setFillColor(sf::Color(0, 0, 255, 150)); 
    block2.setFillColor(sf::Color(255, 0, 0, 150)); 

    block1.setOutlineThickness(3.0f);
    block1.setOutlineColor(sf::Color::White);
    block2.setOutlineThickness(3.0f);
    block2.setOutlineColor(sf::Color::White);

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {}

    sf::Clock deltaClock; 
    float t = 0;

    std::vector<float> time_data;
    std::vector<float> pos_data;
    std::vector<float> vel_data;
    const int MAX_DATA_POINTS = 2000; 

    bool is_simulating = true; 
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Time dtTime = deltaClock.restart();
        ImGui::SFML::Update(window, dtTime);

        float dt = dtTime.asSeconds();
        if (is_simulating) {
            t += dt;
        }

         float a = acceleration(m1, m2, g);
        float T = (2 * m1 * m2 * g) / (m1 + m2); 

        State s = stateAt(t, y0, v0, a);
        float baseY = 300;
        float y1 = baseY + s.y * scale;
        float y2 = baseY - s.y * scale;

        float chao_fisica = 500.0f; 
        float teto = pulleyY + radius;

        if (y1 >= (chao_fisica - block1.getSize().y) || y2 <= teto) {
            is_simulating = false;
        }
        else if (y2 >= (chao_fisica - block2.getSize().y) || y1 <= teto) {
            is_simulating = false;
        }

        float leftRopeX = pulleyX - radius, rightRopeX = pulleyX + radius;
        block1.setPosition(leftRopeX - block1.getSize().x / 2.0f, y1);
        block2.setPosition(rightRopeX - block2.getSize().x / 2.0f, y2);

        sf::Vector2f center1 = block1.getPosition() + block1.getSize() / 2.0f;
        sf::Vector2f center2 = block2.getPosition() + block2.getSize() / 2.0f;

        time_data.push_back(t);
        pos_data.push_back(s.y);
        vel_data.push_back(s.v);

        if (time_data.size() > MAX_DATA_POINTS) {
            time_data.erase(time_data.begin());
            pos_data.erase(pos_data.begin());
            vel_data.erase(vel_data.begin());
        }

        
        ImGui::SetNextWindowPos(ImVec2(550, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(420, 460), ImGuiCond_FirstUseEver);
        ImGui::Begin("Graficos");
        
        if (ImPlot::BeginPlot("Posicao y(t)", ImVec2(-1, 200))) {
            ImPlot::SetupAxes("Tempo (s)", "Posicao (m)");
            ImPlot::PlotLine("y", time_data.data(), pos_data.data(), time_data.size());
            ImPlot::EndPlot();
        }

        if (ImPlot::BeginPlot("Velocidade v(t)", ImVec2(-1, 200))) {
            ImPlot::SetupAxes("Tempo (s)", "Velocidade (m/s)");
            ImPlot::PlotLine("v", time_data.data(), vel_data.data(), time_data.size());
            ImPlot::EndPlot();
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(50, 520), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(900, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Informacoes da Simulacao");
        ImGui::SeparatorText("Parametros");
        ImGui::Text("m1 = %.1f kg  |  m2 = %.1f kg | g = %.1f m/s^2", m1, m2, g);
        ImGui::SeparatorText("Resultados Dinamicos");
        ImGui::Text("Aceleracao (a): %.2f m/s^2", std::abs(a));
        ImGui::SameLine(300);
        ImGui::Text("Tensao na Corda (T): %.2f N", T);
        ImGui::SameLine(600);
        if(!is_simulating) ImGui::TextColored(ImVec4(1,0,0,1), "STATUS: FIM DE CURSO");
        else ImGui::TextColored(ImVec4(0,1,0,1), "STATUS: RODANDO");
        ImGui::End();

        window.clear(sf::Color(30, 30, 30)); 

        float leftX = center1.x;
        float rightX = center2.x;
        sf::Vertex rope1[] = { sf::Vertex({leftX, center1.y}, sf::Color(200,200,200)), sf::Vertex({leftX, pulleyY}, sf::Color(200,200,200)) };
        sf::Vertex rope2[] = { sf::Vertex({rightX, pulleyY}, sf::Color(200,200,200)), sf::Vertex({rightX, center2.y}, sf::Color(200,200,200)) };
        window.draw(rope1, 2, sf::Lines);
        window.draw(rope2, 2, sf::Lines);
        drawArc(window, {pulleyX, pulleyY}, radius);

        window.draw(block1);
        window.draw(block2);
        window.draw(pulley);

        float pSize = 80;
        float vSize = 35;
        drawVector(window, center1, {0, pSize});
        drawVector(window, center2, {0, pSize});

        float dir1 = (s.v >= 0) ? 1.0f : -1.0f;
        float dir2 = -dir1;
        drawVector(window, center1, {0, dir1 * vSize});
        drawVector(window, center2, {0, dir2 * vSize});

        drawLabel(window, font, "m1", center1, 20); 
        drawLabel(window, font, "m2", center2, 20);

        float y_offset_vetor = 15.0f;
        drawLabel(window, font, "P", {center1.x, center1.y + block1.getSize().y/2.0f + y_offset_vetor}, 16);
        drawLabel(window, font, "P", {center2.x, center2.y + block2.getSize().y/2.0f + y_offset_vetor}, 16);
        
        drawLabel(window, font, "v", {center1.x, center1.y - block1.getSize().y/2.0f - y_offset_vetor}, 16);
        drawLabel(window, font, "v", {center2.x, center2.y - block2.getSize().y/2.0f - y_offset_vetor}, 16);

        ImGui::SFML::Render(window);
        
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}