#pragma once

/**
 * @brief The vertex array interface. Each renderer must implement the methods accordingly.
 */

namespace bls
{
    class VertexArray
    {
        public:
            virtual ~VertexArray() { }

            virtual void bind() = 0;
            virtual void unbind() = 0;
    };
};
