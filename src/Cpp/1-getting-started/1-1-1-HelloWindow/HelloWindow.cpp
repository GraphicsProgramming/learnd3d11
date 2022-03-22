#include "HelloWindow.hpp"

#include <string>

HelloWindowApplication::HelloWindowApplication(const std::string_view title)
    : Application(title)
{
}

bool HelloWindowApplication::Load()
{
    return true;
}

void HelloWindowApplication::Render()
{
}

void HelloWindowApplication::Update()
{
}
