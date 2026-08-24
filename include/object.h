/*
 * ================================================================
 * Copyright (c) 2026 Royal Bool - All rights reserved.
 * ================================================================
 */
#ifndef RB_BINARY_OBJECT_H
#define RB_BINARY_OBJECT_H
#include "bmt.h"
#include "platform.h"
#include <string>
#include <type_traits>

#ifndef RB_NO_DEBUG
#define BMT_CHECK(x)                                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        x;                                                                                                             \
        BinaryObject::checkErrors(__FILE__, __LINE__, #x);                                                             \
    } while (0)
#else
#define BMT_CHECK(x) (x)
#endif

namespace rb
{

    /* ================================================================
     * Class BinaryObject
     * ----------------------------------------------------------------
     *
     * ================================================================
     */
    class BinaryObject
    {
        protected:
            /* ================================================================
             * Struct remove_pointer
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            template<typename T>
            struct remove_pointer
            {
                    using type = T;
            };

            /* ================================================================
             * Struct remove_pointer
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            template<typename T>
            struct remove_pointer<T*>
            {
                    using type = typename remove_pointer<T>::type;
            };

            /* ================================================================
             * Struct fundamental_type
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            template<typename T>
            struct fundamental_type
            {
                    using type = typename std::remove_cv<
                        typename std::remove_reference<typename remove_pointer<T>::type>::type>::type;
            };

        public:
            /* ================================================================
             * Method print
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void print()
            {
                bmt_Print(m_head);
            }

        protected:
            /* ================================================================
             * Constructor
             * ================================================================
             */
            BinaryObject();

            /* ================================================================
             * Constructor
             * ================================================================
             */
            BinaryObject(BMT_node node);

            /* ================================================================
             * Destructor
             * ================================================================
             */
            virtual ~BinaryObject();

            /* ================================================================
             * Method checkErrors
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void checkErrors(const BMT_char* file, BMT_uint line, const BMT_char* expression) const;

            /* ================================================================
             * Method listAt
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            static BMT_node listAt(BMT_node list, BMT_index at);

            friend class BTO;

            template<typename T>
            friend class BLO;

            BMT_node m_head;
    };
} // namespace rb
#endif
