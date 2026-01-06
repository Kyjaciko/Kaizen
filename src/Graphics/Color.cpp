////////////////////////////////////////////////////////////////////////////////
// Filename: Color.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Color.h"

Color::Color()
	: m_color(0)
{
}

Color::Color(unsigned int value)
	: m_color(value)
{
}

Color::Color(BYTE r, BYTE g, BYTE b)
	: Color(r, g, b, 255)
{
}

Color::Color(BYTE r, BYTE g, BYTE b, BYTE a)
{
	m_rgba[0] = r;
	m_rgba[1] = g;
	m_rgba[2] = b;
	m_rgba[3] = a;
}

Color::Color(const Color& cpy)
	: m_color(cpy.m_color)
{
}

Color& Color::operator=(const Color& other)
{
	m_color = other.m_color;
	return *this;
}

bool Color::operator==(const Color& other) const
{
	return (this->m_color == other.m_color);
}

bool Color::operator!=(const Color& other) const
{
	return !(*this == other);
}

constexpr BYTE Color::GetR() const
{
	return m_rgba[0];
}

void Color::SetR(BYTE r)
{
	m_rgba[0] = r;
}

constexpr BYTE Color::GetG() const
{
	return m_rgba[1];
}

void Color::SetG(BYTE g)
{
	m_rgba[1] = g;
}

constexpr BYTE Color::GetB() const
{
	return m_rgba[2];
}

void Color::SetB(BYTE b) 
{
	m_rgba[2] = b;
}

constexpr BYTE Color::GetA() const
{
	return m_rgba[3];
}

void Color::SetA(BYTE a)
{
	m_rgba[3] = a;
}