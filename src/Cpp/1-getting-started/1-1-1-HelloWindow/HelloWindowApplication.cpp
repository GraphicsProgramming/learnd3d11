#include "HelloWindowApplication.hpp"

#include <string>

HelloWindowApplication::HelloWindowApplication(const std::string& title)
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
