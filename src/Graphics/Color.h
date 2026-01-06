////////////////////////////////////////////////////////////////////////////////
// Filename: Color.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _COLOR_H_
#define _COLOR_H_

/////////////////
// TYPEDEFINES //
/////////////////

typedef unsigned char BYTE;

////////////////////////////////////////////////////////////////////////////////
// Class name: Color
////////////////////////////////////////////////////////////////////////////////

class Color
{
public:
	Color();
	Color(unsigned int value);
	Color(BYTE r, BYTE g, BYTE b);
	Color(BYTE r, BYTE g, BYTE b, BYTE a);
	Color(const Color& cpy);

	Color& operator=(const Color& other);
	bool operator==(const Color& other) const;
	bool operator!=(const Color& other) const;

	constexpr BYTE GetR() const;
	void SetR(BYTE r);

	constexpr BYTE GetG() const;
	void SetG(BYTE g);

	constexpr BYTE GetB() const;
	void SetB(BYTE b);

	constexpr BYTE GetA() const;
	void SetA(BYTE a);

private:
	union
	{
		BYTE m_rgba[4];
		unsigned int m_color;
	};
};

///////////////
// CONSTANTS //
///////////////

namespace Colors
{
	const Color UnloadedTextureColor(100, 100, 100);
	const Color UnhandledTextureColor(250, 0, 0);
}

#endif // !_COLOR_H_

