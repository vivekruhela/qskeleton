#ifndef UTILS_MEMORY_MANAGER_HPP
#define UTILS_MEMORY_MANAGER_HPP


#include <iostream>
#include <vector>


namespace Utils
{


class MemoryManager
{

public:

    MemoryManager( size_t size, size_t numNewCells = 100 ):
        m_size(size), m_numNewCells(numNewCells) {}
    
    ~MemoryManager()
    {
        // check for memory leaks
        if (m_allocatedMemory.size() * m_numNewCells != m_unusedCells.size())
            std::cout << "Memory leak in memory management system detected: "
                << m_allocatedMemory.size() * m_numNewCells << " bytes allocated, "
                << m_unusedCells.size() << " bytes returned.\n";
        for( size_t i = 0; i < m_allocatedMemory.size(); ++i )
            delete [] m_allocatedMemory[ i ];
    }

    void * newCell()
    {
        // if there are unused cells, get one of it, exclude it from unused and
        // return
        if( m_unusedCells.size() )
        {
            void * result = m_unusedCells.back();
            m_unusedCells.pop_back();
            return result;
        }
        // else try to allocate additional memory, catch exceptions
        else
        {
            try
            {
                char * newPool = new char[m_size * m_numNewCells];
                m_allocatedMemory.push_back(newPool);
                // put cells to vector in order that allows cells with lesser
                // addresses to be used earlier
                for (size_t i = m_numNewCells - 1; i >= 1 ; --i)
                    m_unusedCells.push_back(newPool + m_size * i);
                // return first set in the pool
                return newPool;
            }
            catch(...)
            {
                std::cerr << "Error in memory management system:"
                    << "couldn't allocate additional memory\n";
                return 0;
            }
        }
    }


    void deleteCell(void * pointer)
    {
        // if pointer is not null add to free cells
        if (pointer)
            m_unusedCells.push_back(pointer);
    }

private:

    size_t m_size;
    const size_t m_numNewCells;
    std::vector< void* > m_unusedCells;
    std::vector< char* > m_allocatedMemory;

    // copy is forbidden, no implementation:
    MemoryManager( const MemoryManager& );
    MemoryManager& operator =( const MemoryManager& );

};




template< typename T >
class ArrayMemoryManager
{

public:

    ArrayMemoryManager( size_t numNewCells = 100 ):
          m_numNewCells( numNewCells ),
              m_totalAllocatedBytes(0) {}

    ~ArrayMemoryManager()
{
    // check for memory leaks
    size_t returnedBytes = 0;
    for( size_t i = 0; i < m_sizes.size(); ++i )
        returnedBytes += sizeof(T) * m_sizes[i] *
            m_unusedCells[i].size();
    if (m_totalAllocatedBytes != returnedBytes)
        std::cout << "Memory leak in array memory management system detected:"
            << m_totalAllocatedBytes << " bytes allocated, "
            << returnedBytes << " bytes returned.\n";

    for( size_t i = 0; i < m_allocatedMemory.size(); ++i )
    {
        delete [] m_allocatedMemory[ i ];
    }
}

    T* newArray( size_t numElements )
    {
    // try to find requested numElements among sizes of allocated cells
    for( size_t i = 0; i < m_sizes.size(); ++i )
    {
        if( m_sizes[ i ] == numElements )
        {
            // if there are unused cells of required size, get one of it,
            // exclude it from unused and return
            if( m_unusedCells[i].size() )
            {

                T* result = m_unusedCells[ i ].back();
                m_unusedCells[ i ].pop_back();
                return result;
            }
            // else try to allocate additional memory, catch exceptions
            else
            {
                try
                {
                    T* newPool = new T[ m_numNewCells * numElements ];
                    m_allocatedMemory.push_back( newPool );
                    m_totalAllocatedBytes += sizeof(T) * m_numNewCells * numElements;
                    for( size_t j = 1; j < m_numNewCells; ++j )
                    {
                        m_unusedCells[ i ].push_back( newPool +
                            j * numElements );
                    }
                    return newPool;
                }
                catch(...)
                {
                    std::cerr << "Error in array memory management system:"
                        << "couldn't allocate additional memory\n";
                    std::cerr << "allocated " << m_totalAllocatedBytes << "\n";
                    return 0;
                }
            }
        }
    }

    // if not found add numElements to m_sizes and allocate several cells,
    // catch exceptions
    //std::cerr << "new size = " << numElements << "\n";
    m_sizes.push_back( numElements );
    try
    {
        T* newPool = new T[ m_numNewCells * numElements ];
        m_allocatedMemory.push_back( newPool );
        m_totalAllocatedBytes += sizeof(T) * m_numNewCells * numElements;
        std::vector< T* > memoryPool;
        for( size_t j = 1; j < m_numNewCells; ++j )
        {
            memoryPool.push_back( newPool + j * numElements );
        }
        m_unusedCells.push_back( memoryPool );
        return newPool;
    }
    catch(...)
    {
        std::cout << "Error in array memory management system: couldn't allocate additional memory\n";
        std::cerr << "allocated " << m_totalAllocatedBytes << "\n";
        return 0;
    }
}



    void deleteArray( T* pointer, size_t numElements )
{
    if( pointer )
    {
        for( size_t i = 0; i < m_sizes.size(); ++i )
        {
            if( m_sizes[ i ] == numElements )
            {
                m_unusedCells[i].push_back( pointer );
                return;
            }
        }
    }
}

private:

    const size_t m_numNewCells;
    std::vector< size_t > m_sizes;
    std::vector< std::vector< T* > > m_unusedCells;
    std::vector< T* > m_allocatedMemory;
    size_t m_totalAllocatedBytes;

    // copy is forbidden, no implementation:
    ArrayMemoryManager( const ArrayMemoryManager< T >& );
    ArrayMemoryManager& operator =( const ArrayMemoryManager< T >& );

}; // class ArrayMemoryManager


} // namespace Utils


#endif
