#pragma once

#include <cippie/core/Error.hpp>

#if __has_include(<expected>)
#  include <expected>
#endif

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L

namespace cippie
{
    template <typename T>
    using Result = std::expected<T, Error>;
}

#else

#include <variant>
#include <utility>
#include <stdexcept>
#include <type_traits>

namespace std
{
    template <typename E>
    class unexpected
    {
    public:
        constexpr unexpected() = default;
        constexpr explicit unexpected(const E& e) : m_err(e) {}
        constexpr explicit unexpected(E&& e) : m_err(std::move(e)) {}

        constexpr const E& error() const & noexcept { return m_err; }
        constexpr E& error() & noexcept { return m_err; }
        constexpr const E&& error() const && noexcept { return std::move(m_err); }
        constexpr E&& error() && noexcept { return std::move(m_err); }

    private:
        E m_err;
    };

    template <typename E>
    unexpected(E) -> unexpected<E>;

    template <typename T, typename E>
    class expected
    {
    public:
        constexpr expected() requires std::is_default_constructible_v<T> : m_var(T{}) {}
        constexpr expected(const T& val) : m_var(val) {}
        constexpr expected(T&& val) : m_var(std::move(val)) {}
        constexpr expected(const unexpected<E>& unex) : m_var(unex.error()) {}
        constexpr expected(unexpected<E>&& unex) : m_var(std::move(unex.error())) {}

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return m_var.index() == 0;
        }
        constexpr explicit operator bool() const noexcept {
            return has_value();
        }

        constexpr const T& value() const & {
            if (!has_value()) throw std::runtime_error("bad expected access");
            return std::get<0>(m_var);
        }
        constexpr T& value() & {
            if (!has_value()) throw std::runtime_error("bad expected access");
            return std::get<0>(m_var);
        }
        constexpr T&& value() && {
            if (!has_value()) throw std::runtime_error("bad expected access");
            return std::get<0>(std::move(m_var));
        }

        constexpr const T& operator*() const & noexcept { return std::get<0>(m_var); }
        constexpr T& operator*() & noexcept { return std::get<0>(m_var); }
        constexpr const T&& operator*() const && noexcept { return std::get<0>(std::move(m_var)); }
        constexpr T&& operator*() && noexcept { return std::get<0>(std::move(m_var)); }

        constexpr const T* operator->() const noexcept { return &std::get<0>(m_var); }
        constexpr T* operator->() noexcept { return &std::get<0>(m_var); }

        constexpr const E& error() const & noexcept { return std::get<1>(m_var); }
        constexpr E& error() & noexcept { return std::get<1>(m_var); }
        constexpr E&& error() && noexcept { return std::get<1>(std::move(m_var)); }

    private:
        std::variant<T, E> m_var;
    };

    template <typename E>
    class expected<void, E>
    {
    public:
        constexpr expected() : m_var(std::monostate{}) {}
        constexpr expected(const unexpected<E>& unex) : m_var(unex.error()) {}
        constexpr expected(unexpected<E>&& unex) : m_var(std::move(unex.error())) {}

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return m_var.index() == 0;
        }
        constexpr explicit operator bool() const noexcept {
            return has_value();
        }

        constexpr void value() const {
            if (!has_value()) throw std::runtime_error("bad expected access");
        }

        constexpr const E& error() const & noexcept { return std::get<1>(m_var); }
        constexpr E& error() & noexcept { return std::get<1>(m_var); }
        constexpr E&& error() && noexcept { return std::get<1>(std::move(m_var)); }

    private:
        std::variant<std::monostate, E> m_var;
    };
}

namespace cippie
{
    template <typename T>
    using Result = std::expected<T, Error>;
}

#endif
