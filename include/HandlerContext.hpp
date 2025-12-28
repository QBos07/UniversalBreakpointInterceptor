#pragma once
#include <cstdint>
#include <bit>
#include <functional>

#include "c_handler.h"

namespace UBI {
    template<std::size_t N>
    struct uint_of_size;

    template<>
    struct uint_of_size<1zu> {
        using type = std::uint8_t;
    };

    template<>
    struct uint_of_size<2zu> {
        using type = std::uint16_t;
    };

    template<>
    struct uint_of_size<4zu> {
        using type = std::uint32_t;
    };

    template<std::size_t N>
    using uint_of_size_t = uint_of_size<N>::type;

    template<typename T>
    using size_equal_uint_of = uint_of_size_t<sizeof(T)>;

    template<typename T>
    concept HardwareRegisterCompatible = std::is_trivially_copyable_v<T> && requires
    {
        typename uint_of_size_t<sizeof(T)>;
    };

    class Register {
    public:
        using value_type = uint_of_size_t<sizeof(int)>;

    private:
        value_type value;

        template<HardwareRegisterCompatible Inner>
        class ReadAccessor {
        public:
            using inner_type = Inner;
            using uint_type = size_equal_uint_of<inner_type>;
            using value_type = Register::value_type;

        private:
            const value_type &value;

        public:
            constexpr explicit ReadAccessor(const value_type &value) noexcept : value(value) {
            }

            constexpr operator inner_type() const noexcept {
                return copy();
            }

            constexpr inner_type copy() const noexcept {
                return std::bit_cast<inner_type>(static_cast<uint_type>(value));
            }
        };

        template<HardwareRegisterCompatible Inner>
        class Accessor : public ReadAccessor<Inner> {
        public:
            using base = ReadAccessor<Inner>;
            using inner_type = base::inner_type;
            using uint_type = base::uint_type;
            using value_type = base::value_type;

        private:
            value_type &value;

        public:
            constexpr explicit Accessor(value_type &value) noexcept : base(value), value(value) {
            }

            constexpr Accessor &operator=(const inner_type &other) noexcept {
                value = std::bit_cast<uint_type>(other);
                return *this;
            }

            constexpr Accessor &operator=(const Register &other) noexcept {
                return *this = other.as<inner_type>();
            }

            constexpr Accessor &operator=(const base &other) noexcept {
                return *this = other.copy();
            }

            constexpr Accessor &operator=(base &&other) noexcept {
                return *this = other;
            }

            constexpr Accessor &operator=(const Accessor &other) noexcept {
                return *this = static_cast<const base &>(other);
            }

#define PASSTHROUGH(op) \
            template<typename T> \
            constexpr Accessor &operator op##=(const T &other) { \
                return *this = this->copy() op other; \
            }

            PASSTHROUGH(+)
            PASSTHROUGH(-)
            PASSTHROUGH(*)
            PASSTHROUGH(/)
            PASSTHROUGH(%)
            PASSTHROUGH(<<)
            PASSTHROUGH(>>)
            PASSTHROUGH(|)
            PASSTHROUGH(^)
            PASSTHROUGH(&)

#undef PASSTHROUGH

            constexpr Accessor &operator++() {
                return *this = ++this->copy();
            }

            constexpr inner_type operator++(int) {
                auto tmp = this->copy();
                auto ret = tmp++;
                *this = tmp;
                return ret;
            }

            constexpr Accessor &operator--() {
                return *this = --this->copy();
            }

            constexpr inner_type operator--(int) {
                auto tmp = this->copy();
                auto ret = tmp++;
                *this = tmp;
                return ret;
            }
        };

    public:
        template<HardwareRegisterCompatible T>
        constexpr explicit Register(const T &other) noexcept : value(std::bit_cast<size_equal_uint_of<T> >(other)) {
        }

        template<HardwareRegisterCompatible T>
        constexpr Register &operator=(const T &other) noexcept {
            as<T>() = other;
            return *this;
        }

        template<HardwareRegisterCompatible T>
        constexpr Register &operator=(const Accessor<T> &other) noexcept {
            return *this = other.copy();
        }

        template<HardwareRegisterCompatible T>
        constexpr Accessor<T> as() noexcept {
            return Accessor<T>(value);
        }

        template<HardwareRegisterCompatible T>
        constexpr ReadAccessor<T> as() const noexcept {
            return ReadAccessor<T>(value);
        }

        template<HardwareRegisterCompatible T>
        constexpr T asCopy() const noexcept {
            return as<T>().copy();
        }

        template<HardwareRegisterCompatible T>
        constexpr operator T() const noexcept {
            return asCopy<T>();
        }

        template<HardwareRegisterCompatible T>
        constexpr operator Accessor<T>() noexcept {
            return as<T>();
        }

        template<HardwareRegisterCompatible T>
        constexpr operator ReadAccessor<T>() const noexcept {
            return as<T>();
        }
    };

    class HandlerContext {
        struct Registers {
            Register unbanked[8];
            Register banked[2][8];
            Register SR;
            Register PC;
            Register PR;
            Register MACL;
            Register MACH;
        };

        friend bool ::ubi_c_handler(saved_regs *regs);

        HandlerContext(const saved_regs &) noexcept;

        void writeBack(saved_regs &) const noexcept;

        bool nesting_active = false;

    public:
        Registers registers;
        const Registers original_registers = registers;
        std::reference_wrapper<Register> R[16] = {
            registers.banked[0][0],
            registers.banked[0][1],
            registers.banked[0][2],
            registers.banked[0][3],
            registers.banked[0][4],
            registers.banked[0][5],
            registers.banked[0][6],
            registers.banked[0][7],
            registers.unbanked[0],
            registers.unbanked[1],
            registers.unbanked[2],
            registers.unbanked[3],
            registers.unbanked[4],
            registers.unbanked[5],
            registers.unbanked[6],
            registers.unbanked[7]
        };

        void allowNestedInterrupts() noexcept;
    };

    typedef void (*handler_function)(HandlerContext &);
}
