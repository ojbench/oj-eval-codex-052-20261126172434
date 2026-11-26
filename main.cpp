#include <array>
#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <utility>

using Powers = std::array<int, 3>;

// Standard containers own and release all dynamically allocated term storage.
class Polynomial {
public:
    std::map<Powers, long long, std::greater<Powers>> terms;

    explicit Polynomial(long long value = 0) { add({0, 0, 0}, value); }

    void add(const Powers& powers, long long coefficient) {
        if (coefficient == 0) return;
        auto [it, inserted] = terms.try_emplace(powers, coefficient);
        if (!inserted) it->second += coefficient;
        if (it->second == 0) terms.erase(it);
    }

    Polynomial combine(const Polynomial& other, int sign) const {
        Polynomial result = *this;
        for (const auto& [powers, coefficient] : other.terms)
            result.add(powers, sign * coefficient);
        return result;
    }

    Polynomial operator*(const Polynomial& other) const {
        Polynomial result;
        for (const auto& [a, ca] : terms)
            for (const auto& [b, cb] : other.terms)
                result.add({a[0] + b[0], a[1] + b[1], a[2] + b[2]}, ca * cb);
        return result;
    }

    Polynomial derivative() const {
        Polynomial result;
        for (const auto& [powers, coefficient] : terms) {
            for (int i = 0; i < 3; ++i) {
                if (powers[i] == 0) continue;
                Powers next = powers;
                --next[i];
                if (i == 1) ++next[2];
                if (i == 2) ++next[1];
                result.add(next, coefficient * powers[i] * (i == 2 ? -1 : 1));
            }
        }
        return result;
    }

    bool is_one() const {
        return terms.size() == 1 && terms.begin()->first == Powers{0, 0, 0}
            && terms.begin()->second == 1;
    }

    std::string str() const {
        std::string result;
        for (const auto& [powers, coefficient] : terms) {
            if (coefficient < 0) result += '-';
            else if (!result.empty()) result += '+';
            const long long magnitude = coefficient < 0 ? -coefficient : coefficient;
            if (magnitude != 1 || powers == Powers{0, 0, 0})
                result += std::to_string(magnitude);
            for (int i = 0; i < 3; ++i) {
                if (powers[i] == 0) continue;
                result += i == 0 ? "x" : (i == 1 ? "sin" : "cos");
                if (powers[i] != 1) result += '^' + std::to_string(powers[i]);
                if (i != 0) result += 'x';
            }
        }
        return result.empty() ? "0" : result;
    }
};

struct Fraction {
    Polynomial numerator, denominator;

    Fraction(Polynomial p = Polynomial(), Polynomial q = Polynomial(1))
        : numerator(std::move(p)), denominator(std::move(q)) {}

    Fraction apply(char op, const Fraction& rhs) const {
        if (op == '*') return {numerator * rhs.numerator, denominator * rhs.denominator};
        if (op == '/') return {numerator * rhs.denominator, denominator * rhs.numerator};
        return {(numerator * rhs.denominator).combine(denominator * rhs.numerator,
                                                      op == '+' ? 1 : -1),
                denominator * rhs.denominator};
    }

    Fraction derivative() const {
        return {(numerator.derivative() * denominator).combine(
                    numerator * denominator.derivative(), -1),
                denominator * denominator};
    }

    std::string str() const {
        if (numerator.terms.empty()) return "0";
        std::string p = numerator.str();
        if (denominator.is_one()) return p;
        std::string q = denominator.str();
        if (numerator.terms.size() > 1) p = '(' + p + ')';
        if (denominator.terms.size() > 1) q = '(' + q + ')';
        return p + '/' + q;
    }
};

class Parser {
    const std::string& input;
    std::size_t position = 0;

    char peek() const { return position < input.size() ? input[position] : '\0'; }
    int number() {
        int value = 0;
        while (std::isdigit(static_cast<unsigned char>(peek())))
            value = value * 10 + input[position++] - '0';
        return value;
    }
    int exponent() {
        if (peek() != '^') return 1;
        ++position;
        return number();
    }
    Fraction factor() {
        if (peek() == '-' || peek() == '+') {
            const char sign = input[position++];
            Fraction result = factor();
            if (sign == '-')
                for (auto& term : result.numerator.terms) term.second = -term.second;
            return result;
        }
        if (peek() == '(') {
            ++position;
            Fraction result = expression();
            ++position;
            return result;
        }
        const int coefficient = std::isdigit(static_cast<unsigned char>(peek())) ? number() : 1;
        Powers powers{0, 0, 0};
        // Juxtaposition belongs to a whole monomial, including in a divisor.
        while (peek() == 'x' || peek() == 's' || peek() == 'c') {
            if (peek() == 'x') {
                ++position;
                powers[0] += exponent();
            } else {
                const int index = peek() == 's' ? 1 : 2;
                position += 3;
                powers[index] += exponent();
                ++position; // trailing x in sin^n x or cos^n x
            }
        }
        Polynomial polynomial;
        polynomial.add(powers, coefficient);
        return Fraction(std::move(polynomial));
    }
    Fraction product() {
        Fraction result = factor();
        while (peek() == '*' || peek() == '/') {
            const char op = input[position++];
            result = result.apply(op, factor());
        }
        return result;
    }
public:
    explicit Parser(const std::string& text) : input(text) {}
    Fraction expression() {
        Fraction result = product();
        while (peek() == '+' || peek() == '-') {
            const char op = input[position++];
            result = result.apply(op, product());
        }
        return result;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string input;
    if (!(std::cin >> input)) return 0;
    const Fraction result = Parser(input).expression();
    std::cout << result.str() << '\n' << result.derivative().str() << '\n';
}
