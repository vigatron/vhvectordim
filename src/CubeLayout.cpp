#include "CubeLayout.hpp"

#include <algorithm>
#include <limits>
#include <tuple>

namespace {

struct Candidate {
    CubeLayout::Dimensions dimensions;
    unsigned long long surface = 0;
};

bool candidateLess(const Candidate &lhs, const Candidate &rhs) {
    return std::tie(lhs.surface,
                    lhs.dimensions.height,
                    lhs.dimensions.width,
                    lhs.dimensions.length) <
           std::tie(rhs.surface,
                    rhs.dimensions.height,
                    rhs.dimensions.width,
                    rhs.dimensions.length);
}

} // namespace

std::vector<CubeLayout::Dimensions>
CubeLayout::variants(std::size_t elementCount) {
    std::vector<Dimensions> result;

    if (elementCount < 8 ||
        elementCount > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return result;
    }

    std::vector<Candidate> candidates;

    // Ищем тройку a <= b <= c, где a * b * c == elementCount.
    // Достаточно перебирать a до кубического корня из elementCount.
    for (std::size_t a = 2; a * a * a <= elementCount; ++a) {
        if (elementCount % a != 0) {
            continue;
        }

        const std::size_t rest = elementCount / a;

        for (std::size_t b = a; b * b <= rest; ++b) {
            if (rest % b != 0) {
                continue;
            }

            const std::size_t c = rest / b;
            const unsigned long long surface =
                2ULL * static_cast<unsigned long long>(a * b + a * c + b * c);

            candidates.push_back(Candidate{
                Dimensions{
                    static_cast<int>(a),
                    static_cast<int>(b),
                    static_cast<int>(c)
                },
                surface
            });
        }
    }

    std::sort(candidates.begin(), candidates.end(), candidateLess);
    result.reserve(candidates.size());

    for (const Candidate &candidate : candidates) {
        result.push_back(candidate.dimensions);
    }

    return result;
}

std::optional<CubeLayout::Dimensions> CubeLayout::compute(std::size_t elementCount) {
    const std::vector<Dimensions> all = variants(elementCount);
    if (all.empty()) {
        return std::nullopt;
    }
    return all.front();
}
