#pragma once

class Shader;

class UIElement {
public:
    virtual ~UIElement() = default;
    virtual void draw(Shader& shader) = 0;
    virtual bool handleMouseMoved(double x, double y) = 0;
    virtual bool handleMouseClicked(double x, double y, int button, int action) = 0;
};
