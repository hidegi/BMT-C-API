/*
 * ================================================================
 * Copyright (c) 2026 Royal Bool - All rights reserved.
 * ================================================================
 */
#ifndef RB_BINARY_TREE_OBJECT_H
#define RB_BINARY_TREE_OBJECT_H
#include "blo.h"
#include "object.h"
#include <SFML/System/Lock.hpp>
#include <SFML/System/Mutex.hpp>
#include <algorithm>
#include <cctype>
#include <regex>
#include <string>
#include <vector>

namespace rb
{
    /**
     * Thread-Safety Note:
     * BTO provides mutex-based thread-safety for its member m_head.
     * However, the underlying BMT C API uses thread-local error storage
     * (bmt_error_flag).
     *
     * This means:
     * - Multiple threads can safely access different BTO instances
     * - Error checking via BMT_CHECK macro is thread-safe
     *   (errors are per-thread)
     * - Concurrent access to the same BTO instance is serialized
     *   by m_mutex
     *
     * Known limitation: If bmt functions are called from multiple
     * threads on the same BTO instance rapidly, error state is
     * isolated per thread, not per BTO instance. This is acceptable
     * for diagnostic purposes.
     */
    class RB_API BTO final : public BinaryObject
    {
        public:
            /* ================================================================
             * Constructor
             * ================================================================
             */
            BTO(BMT_node node);

            /* ================================================================
             * Constructor
             * ================================================================
             */
            BTO();

            /* ================================================================
             * Destructor
             * ================================================================
             */
            ~BTO();

            /* ================================================================
             * Constructor
             * ================================================================
             */
            BTO(const BTO& other);
            BTO& operator=(const BTO& other);

            /* ================================================================
             * Constructor
             * ================================================================
             */
            BTO(BTO&& other) noexcept;
            BTO& operator=(BTO&& other) noexcept;

            bool operator==(const BTO& other) const;
            bool operator!=(const BTO& other) const;

            /* ================================================================
             * Method loadFromFile
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            bool loadFromFile(const std::string& filename);

            /* ================================================================
             * Method loadFromMemory
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            bool loadFromMemory(const void* data, size_t size);

            /* ================================================================
             * Method saveToFile
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            bool saveToFile(const std::string& filename) const;

            /* ================================================================
             * Method has
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            bool has(const std::string& domain) const;

            /* ================================================================
             * Method createTree
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void createTree(const std::string& domain);

            /* ================================================================
             * Method createList
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void createList(const std::string& domain);

            template<typename T>

            /* ================================================================
             * Method add
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void add(const std::string& domain, T data);

            template<typename T,
                     typename = typename std::enable_if<std::is_fundamental<typename fundamental_type<T>::type>::value
                                                        && std::is_pointer<T>::value>::type>

            /* ================================================================
             * Method add
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void add(const std::string& domain, BMT_size length, const T data);

            template<typename T>

            /* ================================================================
             * Method set
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            bool set(const std::string& domain, T data);

            template<typename T>

            /* ================================================================
             * Method get
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            T get(const std::string& domain) const;

            /* ================================================================
             * Method remove
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            bool remove(const std::string& domain);

            /* ================================================================
             * Method print
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void print();

            /* ================================================================
             * Method toString
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            std::string toString() const;

        private:
            /* ================================================================
             * Method parseArray
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            bool parseArray(const std::string& expression, std::string& name, std::vector<BMT_size>& v) const;

            /* ================================================================
             * Method isArrayExpression
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            bool isArrayExpression(const std::string& expression) const;

            template<typename InsertFn, typename AppendFn, typename... Args>

            /* ================================================================
             * Method addImpl
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            void addImpl(const std::string& name, InsertFn addFn, AppendFn appendFn, Args&&... args);

            /* ================================================================
             * Method search
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            BMT_node search(const std::string& domain) const;

            /* ================================================================
             * Method search
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            BMT_node search(const std::string& domain, const BMT_node head) const;

            /* ================================================================
             * Method search
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            BMT_node search(std::string_view domain, const BMT_node head) const;

            /* ================================================================
             * Method search
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            BMT_node search(std::string_view domain, const BMT_node head, int depth) const;

            /* ================================================================
             * Method evaluate
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            BMT_node evaluate(const std::string& domain, const BMT_node head) const;

            /* ================================================================
             * Method evaluate
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            BMT_node evaluate(std::string_view domain, const BMT_node head) const;

            /* ================================================================
             * Method remove
             * ----------------------------------------------------------------
             *
             * ================================================================
             */
            bool remove(const std::string& member, BMT_node* object);
            static const std::regex s_array_pattern;
            static const std::regex s_index_pattern;
            mutable sf::Mutex m_mutex;
    };

#include "bto.inl"
} // namespace rb
#endif
