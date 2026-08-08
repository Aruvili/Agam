#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace agam {

/// Compute Levenshtein edit distance between two strings.
inline size_t levenshteinDistance(const std::string &s1, const std::string &s2) {
    const size_t m = s1.size();
    const size_t n = s2.size();
    if (m == 0) return n;
    if (n == 0) return m;

    std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1));
    for (size_t i = 0; i <= m; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= n; ++j) dp[0][j] = j;

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    return dp[m][n];
}

/// Find the closest candidate string for "did you mean '...'?" diagnostic suggestions.
inline std::string findDidYouMean(const std::string &target, const std::vector<std::string> &candidates, size_t maxDistance = 4) {
    std::string bestMatch;
    size_t minDistance = maxDistance;
    for (const auto &cand : candidates) {
        size_t dist = levenshteinDistance(target, cand);
        if (dist < minDistance) {
            minDistance = dist;
            bestMatch = cand;
        }
    }
    return bestMatch;
}

} // namespace agam
