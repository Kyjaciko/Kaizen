////////////////////////////////////////////////////////////////////////////////
// Filename: BasicStructures.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _BASIC_STRUCTURES_H_
#define _BASIC_STRUCTURES_H_

///////////////
// CONSTANTS //
///////////////

namespace Kaizen::Container
{
    struct Rect
    {
        float x, y, w, h;

        float Left() const { return x; }
        float Right() const { return x + w; }
        float Top() const { return y; }
        float Bottom() const { return y + h; }
    };
}

#endif // !_BASIC_STRUCTURES_H_