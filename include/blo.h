/*
 * ================================================================
 * Copyright (c) 2026 Royal Bool - All rights reserved.
 * ================================================================
 */
#ifndef RB_BINARY_LIST_OBJECT_H
#define RB_BINARY_LIST_OBJECT_H
#include "RB/config.h"
#include "bmt.h"
#include "object.h"
#include <SFML/System/Lock.hpp>
#include <SFML/System/Mutex.hpp>
#include <string>
#include <type_traits>
#include <typeinfo>

namespace rb
{
    class BTO;

    /* ================================================================
     * Class BLO
     * ----------------------------------------------------------------
     *
     * ================================================================
     */
    template<typename T>
    class RB_API BLO final : public BinaryObject
    {
        public:
            template<typename U>
            friend class BLO;
            friend class BTO;

            /* ================================================================
             * Constructor
             * ================================================================
             */
            BLO(BMT_node node);

            /* ================================================================
             * Constructor
             * ================================================================
             */
            BLO();

            /* ================================================================
             * Destructor
             * ================================================================
             */
            ~BLO();

            /*
             * virtual bool loadFromFile(const std::string& path) override;
             * virtual bool writeToFile(const std::string& path) override;
             */

            /* ================================================================
             * Constructor
             * ================================================================
             */
            BLO(const BLO& other);
            BLO& operator=(const BLO& other);

            /* ================================================================
             * Constructor
             * ================================================================
             */
            BLO(BLO&& other) noexcept;
            BLO& operator=(BLO&& other) noexcept;

            bool operator==(const BLO<T>& other) const;
            bool operator!=(const BLO<T>& other) const;

            /* ================================================================
             * Class Iterator
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            class Iterator
            {
                private:
                    BMT_node m_current;

                public:
                    /* ================================================================
                     * Constructor
                     * ================================================================
                     */
                    Iterator(BMT_node start)
                        : m_current(start)
                    {
                    }

                    T operator*() const;

                    Iterator& operator++()
                    {
                        if (m_current)
                            m_current = m_current->major;
                        return *this;
                    }

                    bool operator!=(const Iterator& other) const
                    {
                        return m_current != other.m_current;
                    }

                    bool operator==(const Iterator& other) const
                    {
                        return m_current == other.m_current;
                    }
            };

            using const_iterator = Iterator;

            /* ================================================================
             * Method begin
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            Iterator begin()
            {
                return Iterator(m_head);
            }

            /* ================================================================
             * Method end
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            Iterator end()
            {
                return Iterator(nullptr);
            }

            /* ================================================================
             * Method begin
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            const_iterator begin() const
            {
                return const_iterator(m_head);
            }

            /* ================================================================
             * Method end
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            const_iterator end() const
            {
                return const_iterator(nullptr);
            }

            T operator[](BMT_size index) const;

            /* ================================================================
             * Method add
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void add(T t);

            template<typename U,
                     typename = typename std::enable_if<std::is_fundamental<typename fundamental_type<U>::type>::value
                                                        && std::is_pointer<U>::value>::type>

            /* ================================================================
             * Method add
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void add(BMT_size length, const U data);

            /* ================================================================
             * Method set
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void set(BMT_index index, T t);

            /* ================================================================
             * Method get
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            T get(BMT_index index) const;

            /* ================================================================
             * Method remove
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void remove(BMT_index index);

            /* ================================================================
             * Method size
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            BMT_size size() const;

        private:
            BMT_size m_length;
            mutable sf::Mutex m_mutex;
    };

#include "blo.inl"

    using ByteList = BLO<BMT_byte>;
    using ShortList = BLO<BMT_short>;
    using IntList = BLO<BMT_int>;
    using LongList = BLO<BMT_long>;
    using FloatList = BLO<BMT_float>;
    using DoubleList = BLO<BMT_double>;
    using StringList = BLO<std::string>;
    using TreeList = BLO<BTO>;
} // namespace rb
#endif
