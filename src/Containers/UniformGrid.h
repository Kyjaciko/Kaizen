////////////////////////////////////////////////////////////////////////////////
// Filename: UniformGrid.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _UNIFORM_GRID_H_
#define _UNIFORM_GRID_H_

//////////////
// INCLUDES //
//////////////

#include <vector>
#include <algorithm>
#include <functional>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "BasicStructures.h"
#include "../Core/Constants.h"
#include "Helpers/MortonCode2D.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: UniformGrid
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Container
{
    template<typename T>
    class UniformGrid
    {
        struct GridEntry
        {
            T* object = nullptr;
            uint32_t cellIndex = Constant::Container::INDEX_UNINITIALIZED;

            // Required for std::sort.
            [[nodiscard]] bool operator<(const GridEntry& other) const noexcept { return cellIndex < other.cellIndex; }
        };

    public:
        explicit UniformGrid(int width, int height, int cellSize, uint32_t estimatedQuantity = Constant::Container::ESTIMATED_QUANTITY_OF_OBJECTS)
            : m_CellSize(cellSize)
        {
            m_Cols = (width + cellSize - 1) / cellSize;
            m_Rows = (height + cellSize - 1) / cellSize;

            // Standard linear sizing (Rows * Cols) is unsafe here because Z-Curve indexing leaves gaps in the index space on non-square grids.
            // Padding dimensions to the nearest Power-of-Two would solve this but wastes significant memory on wide aspect ratios (e.g. 32:9).
            // We only need to guarantee that the highest generated index fits in memory...
            // So we calculate the Morton code for the bottom-right corner explicitly to determine the strict maximum size required.
            m_CellStart.resize(Morton2D::MortonEncodeLUT(m_Cols - 1, m_Rows - 1) + 1, Constant::Container::INDEX_UNINITIALIZED);
            m_Entries.reserve(estimatedQuantity);
        }

        void Clear() noexcept
        {
            m_Entries.clear();
            std::fill(m_CellStart.begin(), m_CellStart.end(), Constant::Container::INDEX_UNINITIALIZED);
        }

        void Insert(T* obj, const Rect& rect)
        {
            const unsigned int minX = std::max(0, static_cast<int>(rect.Left() / m_CellSize));
            const unsigned int maxX = std::min(m_Cols - 1, static_cast<int>(rect.Right() / m_CellSize));
            const unsigned int minY = std::max(0, static_cast<int>(rect.Top() / m_CellSize));
            const unsigned int maxY = std::min(m_Rows - 1, static_cast<int>(rect.Bottom() / m_CellSize));

            for (unsigned int y = minY; y <= maxY; ++y)
            {
                for (unsigned int x = minX; x <= maxX; ++x)
                {
                    const uint32_t cellIndex = Morton2D::MortonEncodeLUT(x, y);
                    m_Entries.push_back(GridEntry{
                        .object = obj,
                        .cellIndex = cellIndex
                    });
                }
            }
        }

        void Build()
        {
            std::sort(m_Entries.begin(), m_Entries.end());

            for (std::ptrdiff_t i = 0; i < std::ssize(m_Entries); ++i)
            {
                const std::ptrdiff_t cellIdx = static_cast<std::ptrdiff_t>(m_Entries[i].cellIndex);
                if (cellIdx >= std::ssize(m_CellStart) || m_CellStart[cellIdx] != Constant::Container::INDEX_UNINITIALIZED)
                    continue;

                m_CellStart[cellIdx] = static_cast<uint32_t>(i);
            }
        }

        void Query(const Rect& area, std::vector<T*>& outResults) const
        {
            const unsigned int minX = std::max(0, static_cast<int>(area.Left() / m_CellSize));
            const unsigned int maxX = std::min(m_Cols - 1, static_cast<int>(area.Right() / m_CellSize));
            const unsigned int minY = std::max(0, static_cast<int>(area.Top() / m_CellSize));
            const unsigned int maxY = std::min(m_Rows - 1, static_cast<int>(area.Bottom() / m_CellSize));

            for (unsigned int y = minY; y <= maxY; ++y)
            {
                for (unsigned int x = minX; x <= maxX; ++x)
                {
                    const uint32_t cellIdx = Morton2D::MortonEncodeLUT(x, y);

                    // Safety check the index.
                    if (static_cast<size_t>(cellIdx) >= m_CellStart.size())
                        continue;

                    const uint32_t start = m_CellStart[cellIdx];
                    if (start == Constant::Container::INDEX_UNINITIALIZED)
                        continue;

                    // Loop through the entries of this cell.
                    for (std::ptrdiff_t i = start; i < std::ssize(m_Entries); ++i)
                    {
                        if (m_Entries[i].cellIndex != cellIdx) 
                            break;

                        T* obj = m_Entries[i].object;

                        // Prevent duplicate additions.
                        // If the object is for example in two cells that we check.
                        if (std::find(outResults.begin(), outResults.end(), obj) != outResults.end())
                            continue;
                        
                        outResults.push_back(obj);
                    }
                }
            }
        }

        void CheckCollisions(const std::function<void(T*, T*)>& onCollision) const
        {
            // Loop over each cell.
            for (std::ptrdiff_t index = 0; index < std::ssize(m_CellStart); ++index)
            {
                const uint32_t start = m_CellStart[index];
                if (start == Constant::Container::INDEX_UNINITIALIZED)
                    continue;

                // Search for the end of the cell in Array L.
                std::ptrdiff_t end = static_cast<std::ptrdiff_t>(start);
                while (end < std::ssize(m_Entries) && static_cast<std::ptrdiff_t>(m_Entries[end].cellIndex) == index)
                    ++end;

                // Couple each object with every other object in the cell.
                for (std::ptrdiff_t i = start; i < end; ++i)
                {
                    for (std::ptrdiff_t j = i + 1; j < end; ++j)
                    {
                        onCollision(m_Entries[i].object, m_Entries[j].object);
                    }
                }
            }
        }

    private:
        int m_CellSize = 0;
        int m_Cols = 0, m_Rows = 0;

        std::vector<uint32_t>   m_CellStart; // Array C: Contains the start index in m_Entries for each cell.
        std::vector<GridEntry>  m_Entries;   // Array L: Contains all object references, sorted by cell index.
    };
}

#endif // !_UNIFORM_GRID_H_